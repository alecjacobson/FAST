#ifndef COLLISION_SPHERES_H
#define COLLISION_SPHERES_H
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <utility>
#include <vector>
#include <map>


//the sphere struct with center and radius
typedef struct Sphere
{
    Eigen::Vector3d center;
    double squared_radius;
    double radius;
} Sphere;

//the tree structure containing 'Sphere's
class Spheres
{
  public:
  static std::map<Spheres*, std::vector<int> > vertex_map;
  // Map to M times mass matrix  (see lbs_matrix and arap_dof)
  static std::map<Spheres*, Eigen::MatrixXd > M_map;
    //the sphere itself
    Sphere sphere;
    
    Sphere deformed_sphere;
    
    //the children
    Spheres* left;
    Spheres* right;
    
    //the parent
    Spheres* parent;
    
    //may come in handy
    int level;
    
    //The "lazy" n-ary tree: If a sphere has a similar size to its parent, it will be pruned, i.e. jump directly to its children if this is the case
    bool is_pruned;
    
    bool is_colliding;
    
    //The joint sets as described in the paper
    //1-0 matrix with size #joint_sets by #handles 
    //Only the ones containing a 1 are in the joint set
    Eigen::MatrixXd joint_sets;
    
    //the corners as described in the paper
    //the vector has #joint sets elements
    //each matrix has size #corners by #handles and keeps the corners corrsponding to that joint set
    std::vector<Eigen::MatrixXd> corners;
};

typedef std::pair<Spheres*, Spheres*> SpheresPair;
typedef std::vector<SpheresPair> SpheresPairVector;
typedef std::vector<Spheres*> SpheresVector;

// Inputs:
//   V  #V by dim list of vertex rest positions
//   F  #F by {3|4} list of {triangle|tet} indices
//   W  #V by #handles list of correspondence weights
// Outputs:
//    S   Spheres according to Ladislav's paper
template <typename LbsMatrixType>
void construct_spheres(
  const Eigen::MatrixXd & V,
  const Eigen::MatrixXi & F,
  const Eigen::MatrixXd & W,
  const Eigen::SparseMatrix<double> & Mass,
  const LbsMatrixType & M,
  Spheres &S,
  int levels);

// Inputs:
//   S  Spheres class (see construct_spheres)
//   T  #handles * dim * dim+1 list of transformation entries (see e.g. arap_dof.h)
// Outputs:
//   SP  List of pairs of spheres that are in contact
void contact_spheres(
                     Spheres & S,
                     SpheresPairVector & SP);

void contact_floor(
                   Spheres & S,
                   double floorY,
                   SpheresVector & SC);

void contact_set_all_false(Spheres & S);

void contact_spheres_recursive(
                            Spheres & S1,
                            Spheres & S2,
                            SpheresPairVector & SP);

void contact_floor_recursive(
                        Spheres & S,
                        double floorY,
                        SpheresVector & SC);




// Inputs:
//   S  Spheres class to be deleted
void delete_spheres(Spheres & S);

// Return false if indices do not match T
bool deform_spheres(
                    Spheres & S,
                    const Eigen::MatrixXd & T);
#endif
