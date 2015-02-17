#include "collision_spheres.h"
#include "render_spheres.h"
//the implementation by Bernd Gaertner
#include "Miniball.h"
#include "Tform.h"

#include <igl/repdiag.h>
#include <limits>
#include <iostream>



#define WEIGHT_TOLERANCE 1e-3
#define SUM_TOLERANCE 1e-3

//the dimension is constant, 3
const int d = 3;
std::map<Spheres*, std::vector<int> > Spheres::vertex_map;
std::map<Spheres*, Eigen::MatrixXd > Spheres::M_map;
// Inputs:
//   points  The points that should be inside the sphere
// Outputs:
//    S   Spheres according to Ladislav's paper
Sphere compute_min_sphere(const Eigen::MatrixXd &points)
{
    //The miniball interface
    Miniball<d> mb;
    
    //the Point class used in Miniball
    Point<d> p;
    
    //check in the points
    for(int i = 0; i < (int)points.rows(); ++i)
    {
        p[0] = points(i,0);
        p[1] = points(i,1);
        p[2] = points(i,2);
        
        mb.check_in(p);
    }    
    
    //compute the spehere
    mb.build();
    
    //get the center and the squared_radius
    p = mb.center();
    double rad = mb.squared_radius();
    
    //copy these values
    Sphere result;
    result.center(0) = p[0];
    result.center(1) = p[1];
    result.center(2) = p[2];
    
    result.squared_radius = rad;
    result.radius = sqrt(rad);
    
    return result;
}

void sphere_precomputations(
                            Spheres* root,
                            const Eigen::MatrixXd & /*V*/,
                            const Eigen::MatrixXd & W)
{
    std::vector<Eigen::VectorXd> my_vec;
    
    int no_handles = W.cols();
    
    double weight_tolerance = WEIGHT_TOLERANCE;
    
    //count the number of seperate joint sets    
    for(int i = 0; i < (int)W.rows(); ++i)
    {
        Eigen::VectorXd cur = W.row(i);
        for(int j = 0; j < (int)no_handles; ++j)
            cur(j) = (cur(j) > weight_tolerance) * 1.0;
        
        //check whether this vector already exists
        if(my_vec.empty())
        {
            my_vec.push_back(cur);
            continue;
        }
        
        bool found = false;
        for(int k = 0; k < (int)my_vec.size(); ++k)
        {
            found = true;
            for(int j = 0; j < (int)no_handles; ++j)
            {
                if(my_vec[k](j) != cur(j))
                {
                    found = false;
                    break;
                }
            }
            if(found)
                break;
        }
        if(found)
            continue;
        
        my_vec.push_back(cur);
    }
    
    //save the joint sets
    root->joint_sets = Eigen::MatrixXd::Zero(my_vec.size(), W.cols());
    
    for(int i = 0; i < (int)my_vec.size(); ++i)
        for(int j = 0; j < (int)W.cols(); ++j)
            root->joint_sets(i,j) = my_vec[i](j);
    
    //compute the weight intervals
    Eigen::MatrixXd intervals_low = Eigen::MatrixXd::Ones(my_vec.size(), no_handles);
    Eigen::MatrixXd intervals_high = Eigen::MatrixXd::Zero(my_vec.size(), no_handles);
    
    //go over all the weights again and compute the lower and upper bounds
    for(int i = 0; i < (int)W.rows(); ++i)
    {
        Eigen::VectorXd cur = W.row(i);
        for(int j = 0; j < (int)no_handles; ++j)
            cur(j) = (cur(j) > weight_tolerance) * 1.0;
        
        //check whether this vector already exists
        int index = 0;
        bool found = false;
        for(int k = 0; k < (int)my_vec.size(); ++k)
        {
            found = true;
            for(int j = 0; j < (int)W.cols(); ++j)
            {
                if(my_vec[k](j) != cur(j))
                {
                    found = false;
                    break;
                }
            }
            if(found)
            {
                index = k;
                break;
            }
        }
        assert(found);
        
        //correct the bounds of the k'th joint set
        for(int j = 0; j < (int)W.cols(); ++j)
        {
            if(my_vec[index](j) == 0)
                continue;
            if(W(i,j) < intervals_low(index,j))
                intervals_low(index,j) = W(i,j);
            
            if(W(i,j) > intervals_high(index,j))
                intervals_high(index,j) = W(i,j);
        }
    }
    
    my_vec.clear();
    
    //compute the corners according to the paper
    root->corners.resize(intervals_low.rows());
    
    double sum_tolerance = SUM_TOLERANCE;
    //iterate over all joint sets
    for(int k = 0; k < (int)intervals_low.rows(); ++k)
    {
        //get the non-zero elements inside the joint set
        std::vector<int> elements;
        for(int i = 0; i < (int)root->joint_sets.cols(); ++i)
        {
            if(root->joint_sets(k, i) > 0)
                elements.push_back(i);
        }
        
        //go with "leave one out" and compute its value(s)
        for(int i = 0; i < (int)elements.size(); ++i)
        {
            //leave cur_el out
            int cur_el = elements[i];
            
            //for computing the probabilities (choose from low or high interval end)
            std::vector<int> remainder(elements.size());
            for(int j = 0; j < (int)remainder.size(); ++j) 
                remainder[j] = 0;
            
            //iterate until remainder is all 0 again, i.e. check all combinations
            int ind_sum = 0;
            bool first_time = true;
            while(first_time || ind_sum > 0)
            {
                first_time = false;
                //compute current value for cur_ind
                double sum = 0;
                for(int j = 0; j < (int)remainder.size(); ++j)
                {
                    if(j == i)
                        continue;
                    if(remainder[j] == 0)
                        sum += intervals_low(k, elements[j]);
                    else 
                        sum += intervals_high(k, elements[j]);
                }
                
                //accept point if this is the case
                double val = 1 - sum;
                if(val <= intervals_high(k, cur_el) + sum_tolerance && val >= intervals_low(k, cur_el) - sum_tolerance)
                {
                    Eigen::VectorXd cur = Eigen::VectorXd::Zero(W.cols());
                    for(int j = 0; j < (int)remainder.size(); ++j)
                    {
                        if(j == i)
                            cur(cur_el) = val;
                        else if(remainder[j] == 0)
                            cur(elements[j]) = intervals_low(k, elements[j]);
                        else 
                            cur(elements[j]) = intervals_high(k, elements[j]); 
                    }
                    my_vec.push_back(cur);
                }
                
                //compute new sum
                ind_sum = (ind_sum + 1) % (int)pow(2.0, (double)remainder.size() - 1);
                
                int cur_sum = ind_sum;
                //compute new combination from sum
                for(int j = 0; j < (int)remainder.size(); ++j)
                {
                    if(i == j)
                        continue;
                    remainder[j] = cur_sum % 2;
                    cur_sum /= 2;
                }
            }
        }
        
        if(my_vec.size() == 0)
        {
            sum_tolerance *= 2;
            --k;
            continue;
        }
        
        sum_tolerance = SUM_TOLERANCE;
        
        //save the corners
        root->corners[k] = Eigen::MatrixXd::Zero(my_vec.size(), W.cols());
        for(int i = 0; i < (int)my_vec.size(); ++i)
            for(int j = 0; j < (int)W.cols(); ++j)
                root->corners[k](i,j) = my_vec[i](j);
        
        my_vec.clear();
    }
}

// Inputs:
//   root  Spheres class (see construct_spheres)
//   V  #V by dim list of vertex rest positions inside the sphere
//   int level the level at which we are
//   int max_level the maximum depth
// Outputs:
//   root  constructed Sphere
template <typename LbsMatrixType>
void create_tree(
  Spheres* root, 
  const Eigen::MatrixXd & V, 
  const Eigen::MatrixXi & V_ind, 
  const Eigen::MatrixXi & F, 
  const Eigen::MatrixXd & W, 
  const LbsMatrixType & MM,
  int level, 
  int max_level)
{
  using namespace std;
  const int n = V.rows();

    if(root->parent != NULL)
    {
        //get the vertex indices
        Eigen::MatrixXd M_root(Eigen::MatrixXd::Zero(MM.rows(),3));

        //Eigen::MatrixXd indicator(Eigen::MatrixXd::Zero(n*3,3));
        vector<int> cur_vertices;
        for(int i = 0; i < (int)V_ind.rows(); ++i)
        {
            if(V_ind(i) == 1)
            {
                cur_vertices.push_back(i);
                //indicator(V_ind(i),0) = 1;
                //indicator(V_ind(i)+n*1,1) = 1;
                //indicator(V_ind(i)+n*2,2) = 1;
                M_root.col(0).array() += MM.col(i+0*n).array();
                M_root.col(1).array() += MM.col(i+1*n).array();
                M_root.col(2).array() += MM.col(i+2*n).array();
            }
        }
        //cout<<"M_root=["<<M_root<<endl<<"];"<<endl;
        //M_root = MM*indicator;
        assert(M_root.cols() == 3);
        assert(M_root.rows() == MM.rows());
        
        //add it to the map
        Spheres::vertex_map.insert( pair<Spheres*, vector<int> >(root, cur_vertices) );
        Spheres::M_map.insert( pair<Spheres*, Eigen::MatrixXd>(root,M_root));
    }

    //stopping criteria    
    if(level == max_level)
    {
        root->left = NULL;
        root->right = NULL;
        return;
    }
    
    /*subdivide the root-tree into two parts*/
    //find where to subdivide it (mean of the dimension with max change)
    double min_coord[3] = {2e26, 2e26, 2e26};
    double max_coord[3] = {-2e26, -2e26, -2e26};
    double mean_coord[3] = {0};
    
    int no_el = V_ind.sum();
    for(int i = 0; i < (int)V.rows(); ++i)
    {
        if(V_ind(i) == 0)
            continue;
        for(int j = 0; j < 3; ++j)
        {
            double val = V(i,j);
            if(min_coord[j] > val)
                min_coord[j] = val;
            
            if(max_coord[j] < val)
                max_coord[j] = val;
            
            mean_coord[j] += val;
        }
    }
    mean_coord[0] /= no_el;
    mean_coord[1] /= no_el;
    mean_coord[2] /= no_el;
    
    
    //find the largest changing dimension
    int max = 0;
    double max_change = max_coord[0] - min_coord[0];
    for(int j = 1; j < 3; ++j)
    {
        if(max_coord[j] - min_coord[j] > max_change)
        {
            max = j;
            max_change = max_coord[j] - min_coord[j];
        }
    }
    
    //count l and r according to the faces
    int l = 0, r = 0;
    for(int i = 0; i < (int)F.rows(); ++i)
    {
        //compute the mean of the face
        int v1 = F(i,0);
        int v2 = F(i,1);
        int v3 = F(i,2);
        
        double mean = V(v1,max) + V(v2, max) + V(v3, max);
        mean /= 3.0;
        
        if(mean < mean_coord[max])
            ++l;
        else
            ++r;
    }
    
    //the faces on both sides
    Eigen::MatrixXi faces_l(l,3);
    Eigen::MatrixXi faces_r(r,3);
    
    //the vertex indices on both sides;
    Eigen::VectorXi point_ind_l = Eigen::VectorXi::Zero(V.rows());
    Eigen::VectorXi point_ind_r = Eigen::VectorXi::Zero(V.rows());
    
    l = 0, r = 0;
    for(int i = 0; i < (int)F.rows(); ++i)
    {
        //compute the mean of the face
        int v1 = F(i,0);
        int v2 = F(i,1);
        int v3 = F(i,2);
        
        double mean = V(v1,max) + V(v2, max) + V(v3, max);
        mean /= 3.0;
        
        if(mean < mean_coord[max])
        {
            //put the faces
            faces_l(l,0) = v1;
            faces_l(l,1) = v2;
            faces_l(l,2) = v3;
            
            //put the vertices
            point_ind_l(v1) = 1;
            point_ind_l(v2) = 1;
            point_ind_l(v3) = 1;
            
            ++l;
        }
        else
        {
            //put the faces
            faces_r(r,0) = v1;
            faces_r(r,1) = v2;
            faces_r(r,2) = v3;
            
            //put the vertices
            point_ind_r(v1) = 1;
            point_ind_r(v2) = 1;
            point_ind_r(v3) = 1;
            
            ++r;
        }
    }

    //compute the number of vertices in r and l
    l = point_ind_l.sum();
    r = point_ind_r.sum();
    
    //put the nodes and weights on both sides into matrices   
    Eigen::MatrixXd points_l(l,3);
    Eigen::MatrixXd points_r(r,3);
    
    int no_weights = W.cols();
    Eigen::MatrixXd weights_l(l, no_weights);
    Eigen::MatrixXd weights_r(r, no_weights);
    
    l = 0; r = 0;
    for(int i = 0; i < (int)V.rows(); ++i)
    {
        if(point_ind_l(i) > 0)
        {
            points_l(l,0) = V(i,0);
            points_l(l,1) = V(i,1);
            points_l(l,2) = V(i,2);
            
            for(int j = 0; j < no_weights; ++j)
                weights_l(l,j) = W(i,j);
            
            ++l;
        }
        if(point_ind_r(i) > 0)
        {
            points_r(r,0) = V(i,0);
            points_r(r,1) = V(i,1);
            points_r(r,2) = V(i,2);
            
            for(int j = 0; j < no_weights; ++j)
                weights_r(r,j) = W(i,j);
            
            ++r;
        }
    }
    
    //if l and are too small, don't bother computing the next level
    if(l < 3 || r < 3)
    {
        root->left = NULL;
        root->right = NULL;
        return;
    }
        
    //compute the minimum spheres on both sides
    Sphere minimum_l = compute_min_sphere(points_l);
    Sphere minimum_r = compute_min_sphere(points_r);
    
    root->left = new Spheres();
    root->right = new Spheres();
    
    root->left->sphere = minimum_l;
    root->right->sphere = minimum_r;    
    
    //levels
    root->left->level = level;
    root->right->level = level;
    
    //back pointers
    root->left->parent = root;
    root->right->parent = root;
    
    //not pruned(yet)
    root->left->is_pruned = false;
    root->right->is_pruned = false;
    
    root->left->is_colliding = false;
    root->right->is_colliding = false;
    
    //make the precomputations    
    sphere_precomputations(root->left, points_l, weights_l);
    sphere_precomputations(root->right, points_r, weights_r);
    
    
    //subdivide left child
    create_tree(root->left, V, point_ind_l, faces_l, W, MM,  level + 1, max_level);
    
    //subdivide right child
    create_tree(root->right, V, point_ind_r, faces_r, W, MM, level + 1, max_level);
}

// Inputs:
//   root  The tree to be pruned
// Outputs:
//   root  lazy pruned Sphere tree
void prune_tree(Spheres *root)
{
    //can't prune NULL, backtrack
    if(root == NULL)
        return;

    // nor leaves
    if(root->left == NULL && root->right == NULL)
        return;
    
    //can't prune root, go on
    if(root->parent == NULL)
    {
        prune_tree(root->left);
        prune_tree(root->right);
    }
    else
    {
        //check whether root is "prunable", check closest non-pruned parent
        Spheres* papa = root->parent;
        while(papa->is_pruned)
            papa = papa->parent;
        
        //compute the ratio
        double ratio = root->sphere.squared_radius / papa->sphere.squared_radius;
        
        //check pruning condition
        if(ratio > 0.6)
        {
            root->is_pruned = true;
//            //update the levels
//            if(root->left != NULL)
//                root->left->level = papa->level + 1;
//            if(root->right != NULL)
//                root->right->level = papa->level + 1;
        }
        
        //prune the children
        prune_tree(root->left);
        prune_tree(root->right);
    }
}

template <typename LbsMatrixType>
void construct_spheres(
  const Eigen::MatrixXd & V,
  const Eigen::MatrixXi & F,
  const Eigen::MatrixXd & W,
  const Eigen::SparseMatrix<double> & Mass,
  const LbsMatrixType & M,
  Spheres &S,
  int levels)
{
  using namespace std;
  using namespace igl;
  using namespace Eigen;
  //compute sphere with all points
    Sphere minimum = compute_min_sphere(V);
    
    S.sphere = minimum;
    
    S.parent = NULL;
    
    S.is_pruned = false;
    
    S.is_colliding = false;
    
    S.level = 0;
    
    Eigen::VectorXi point_ind = Eigen::VectorXi::Ones(V.rows());
    SparseMatrix<double> Mass_rep;
    repdiag(Mass,3,Mass_rep);
    Eigen::MatrixXd MM = (Mass_rep *  M).transpose().eval();
    assert(MM.rows() == M.cols());
    assert(MM.cols() == V.rows()*3);
    create_tree( &S, V, point_ind, F, W, MM, 1, levels);
    
    //prune_tree(&S);
    
    cout<<"Tree with "<<levels<<" levels"<<endl;
}

void delete_sphere(Spheres *root)
{
    if(root->left == NULL && root->right == NULL)
    {
        //don't delete root, it's not a pointer!
        if(root->parent != NULL)
        {
            delete root;
        }
    }
    else
    {
        delete_sphere(root->left);
        delete_sphere(root->right);
        
        //don't delete root, it's not a pointer!
        if(root->parent != NULL)
        {
            delete root;
        }
    }
}

void delete_spheres(Spheres &S)
{
    delete_sphere(&S);
    Spheres::vertex_map.clear();
}

bool deform_sphere(Spheres* root, const Eigen::MatrixXd & T)
{
    if(root == NULL)
    {
        return true;
    }

    
    //don't compute it for pruned vertices
    //if(!root->is_pruned && !(root -> parent == NULL))
    if(!root->is_pruned && !(root -> parent == NULL))
    {    
      std::vector<Eigen::Vector3d> centers;
      //compute the mapping of the spheres for all joint sets
      for(int k = 0; k < (int)root->joint_sets.rows(); ++k)
      {
          std::vector<int> indices;
          //find the transformations that are valid inside the current joint set
          for(int i = 0; i < (int)root->joint_sets.cols(); ++i)
          {
              if(root->joint_sets(k, i) > 0)
                  indices.push_back(i);
          }
          
          std::vector<Eigen::Vector3d> cur_centers;
          //// Number of weights/transformations
          //const int m  = T.cols()/4;
          //compute the new center of S_{i,1} to S_{i,p_i} by multiplying with the transformations
          for(int i = 0; i < (int)indices.size(); ++i)
          {
            
              Eigen::MatrixXd Tid = T.block(0,indices[i]*4,3,4);
            Tform3 Ti(Tform3::Identity());
            Ti.matrix().block(0,0,3,4) = (Tid);
//            Tform3 Ti(Tform3::Identity());
            cur_centers.push_back(Ti * root->sphere.center);
//            cur_centers.push_back(root->sphere.center);
          }
          
          //Use equation 5 to compute the R_i
          for(int i = 0; i < (int)root->corners[k].rows(); ++i)
          {
              Eigen::Vector3d new_center = Eigen::Vector3d::Zero();
              for(int ind = 0; ind < (int)indices.size(); ++ind)
              {
                  new_center += root->corners[k](i, indices[ind]) * cur_centers[ind];
              }
              centers.push_back(new_center);
          }
      }

      //Put all Rij together and compute the new bounding sphere, the new center is their mean
      //go thru centers to make the computations, save the new sphere under root->deformed_sphere
      //The radius stays the same for all, should be approximate
      Eigen::Vector3d centroid(0,0,0);
      for(int i = 0; i < (int)centers.size(); ++i)
      {
          //compute mean of centers
          centroid.array() += centers[i].array();
          
      }

        centroid /= (double)centers.size();
        
      //compute bounding box of the spheres given the global radius in root->sphere.radius

      // Loop over cetners finding farthest to centroid
#ifndef NDEBUG
      int max_i = -1;
#endif
      double max_d = -1e26;
      for(int i = 0; i < (int)centers.size(); ++i)
      {
          //TO-DO implement here
        double d = (centers[i]-centroid).norm();
        if(d>max_d)
        {
#ifndef NDEBUG
          max_i = i;
#endif
          max_d = d;
        }
      }
      assert(max_i>=0);

        root->deformed_sphere.center = centroid;
//        root->deformed_sphere.radius = root->sphere.radius;
      root->deformed_sphere.radius = max_d + root->sphere.radius;

    }

    //compute the mappings of the children
    bool left_ret = deform_sphere(root->left, T);
    bool right_ret = deform_sphere(root->right, T);
    return right_ret && left_ret;
}

bool deform_spheres(
                    Spheres & S,
                    const Eigen::MatrixXd & T)
{
    
    return deform_sphere(&S, T);
}

void contact_spheres(
                     Spheres & S,
                     SpheresPairVector & /*SP*/)
{
    //TODO: Implement sphere contacts and apply forces
    //std::cout<<"contact_spheres()"<<std::endl;
    if (S.left != NULL && S.right != NULL)
    {
        //contact_spheres_recursive(*S.left, *S.right, SP);
    }
    
    //SpheresVector SC;
    //contact_floor_recursive(S, 0., SC);
}


void contact_floor(
                   Spheres & S,
                   double floorY,
                   SpheresVector & SC)
{
    //std::cout<<"contact_floor()"<<std::endl;
    contact_set_all_false(S);
    if (S.left != NULL)
    {
      contact_floor_recursive(*S.left, floorY, SC);
    }
    if (S.right != NULL)
    {
      contact_floor_recursive(*S.right, floorY, SC);
    }
}


void contact_set_all_false(
                            Spheres & S)
{
    S.is_colliding = false;
    if (S.left != NULL)
    {
        contact_set_all_false(*S.left);
    }
    if (S.right != NULL)
    {
        contact_set_all_false(*S.right);
    }
}


void contact_spheres_recursive(
                         Spheres & S1,
                         Spheres & S2,
                         SpheresPairVector & SP) 
{
    double dist = (S1.deformed_sphere.center - S2.deformed_sphere.center).norm();
    
    if (dist < (S1.deformed_sphere.radius + S2.deformed_sphere.radius)) // S1 and S2 are intersecting
    {
        //cout << "Spheres are intersecting" << endl;
        if (S1.left != NULL && S1.right != NULL && S2.left != NULL && S2.right != NULL) // We have not reached a leaf
        {
            //cout << "We have NOT reached a leaf" << endl;
            contact_spheres_recursive(*S1.left, *S1.left, SP);
            contact_spheres_recursive(*S1.right, *S1.right, SP);
            contact_spheres_recursive(*S1.right, *S1.left, SP);
            
            contact_spheres_recursive(*S2.right, *S2.right, SP);
            contact_spheres_recursive(*S2.left, *S2.left, SP);
            contact_spheres_recursive(*S2.right, *S2.left, SP);
            
            contact_spheres_recursive(*S1.right, *S2.right, SP);
            contact_spheres_recursive(*S1.right, *S2.left, SP);
            contact_spheres_recursive(*S1.left, *S2.right, SP);
            contact_spheres_recursive(*S1.left, *S2.left, SP);
        }
        else // We have reached a leaf
        {
            //cout << "We have reached a leaf" << endl;
            if (&S1 != &S2) 
            {
                SP.push_back(SpheresPair(&S1, &S2));
            }
        }
    } 
    else // S1 and S2 are not intersecting
    {
        //cout << "Spheres are NOT intersecting" << endl;
        if (S1.left != NULL && S1.right != NULL && S2.left != NULL && S2.right != NULL) // We have not reached a leaf
        {
            //cout << "We have NOT reached a leaf" << endl;
            contact_spheres_recursive(*S1.left, *S1.left, SP);
            contact_spheres_recursive(*S1.right, *S1.right, SP);
            contact_spheres_recursive(*S1.right, *S1.left, SP);
            
            contact_spheres_recursive(*S2.right, *S2.right, SP);
            contact_spheres_recursive(*S2.left, *S2.left, SP);
            contact_spheres_recursive(*S2.right, *S2.left, SP);
        }
    }
}


void contact_floor_recursive(
                        Spheres & S,
                        double floorY,
                        SpheresVector & SC)
{
    //std::cout<<"contact_floor_recursive()"<<std::endl;
    //double dist = S.deformed_sphere.center.y() - floorY;
    
    //if (dist < S.deformed_sphere.radius) // S is intersecting the floor
    //std::cout<<"S.sphere.center.y() ("<< S.sphere.center.y()<<") floorY ("<< floorY<<" S.sphere.radius ("<<S.sphere.radius<<")"<<std::endl;
    //std::cout<<"S.deformed_sphere.center.y() ("<< S.deformed_sphere.center.y()<<") floorY ("<< floorY<<" S.deformed_sphere.radius ("<<S.deformed_sphere.radius<<")"<<std::endl;
    if(S.deformed_sphere.center.y() < (floorY + S.deformed_sphere.radius))
    {
        //cout << "We have an intersection" << endl;
        S.is_colliding = true;
        if (S.left != NULL )// We have not reached a leaf
        {
            contact_floor_recursive(*S.left, floorY, SC);
        }
        if (S.right != NULL )// We have not reached a leaf
        {
            contact_floor_recursive(*S.right, floorY, SC);
        }
        else // We have reached a leaf
        {
            SC.push_back(&S);
        }
    }else
    {
      //std::cout<<"level: "<<S.level<<std::endl;
        //if (S.left != NULL && S.right != NULL) // We have not reached a leaf
        //{
        //    contact_floor_recursive(*S.right, floorY, SC);
        //    contact_floor_recursive(*S.left, floorY, SC);
        //}
    }
}

// Explicit instanciation
template void construct_spheres<Eigen::Matrix<double, -1, -1, 0, -1, -1> >(Eigen::Matrix<double, -1, -1, 0, -1, -1> const&, Eigen::Matrix<int, -1, -1, 0, -1, -1> const&, Eigen::Matrix<double, -1, -1, 0, -1, -1> const&, Eigen::SparseMatrix<double, 0, int> const&, Eigen::Matrix<double, -1, -1, 0, -1, -1> const&, Spheres&, int);
