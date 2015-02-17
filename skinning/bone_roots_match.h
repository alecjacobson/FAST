// Check whether 2 sets of bone roots contain the same tree structure
// Inputs:
//   AR first set of bone roots
//   BR second set of bone roots
bool bone_roots_match(
  const std::vector<Bone*> & AR,
  const std::vector<Bone*> & BR);

// Implementation

bool bone_roots_match(
  const std::vector<Bone*> & AR,
  const std::vector<Bone*> & BR)
{
  using namespace std;
  if(AR.size() != BR.size())
  {
    // different number of roots
    return false;
  }
  // Insert roots into search queue
  std::list<Bone*> AQ;
  std::list<Bone*> BQ;
  for(
    std::vector<Bone*>::const_iterator ait = AR.begin();
    ait != AR.end();
    ait++)
  {
    AQ.push_back(*ait);
  }
  for(
    std::vector<Bone*>::const_iterator bit = BR.begin();
    bit != BR.end();
    bit++)
  {
    BQ.push_back(*bit);
  }
  assert(AQ.size() == BQ.size());

  // Keep track of all bones that get popped 
  std::vector<Bone*> A;
  std::vector<Bone*> B;
  while(!AQ.empty() && !BQ.empty())
  {
    assert(AQ.size() == BQ.size());
    Bone * a = AQ.back();
    AQ.pop_back();
    Bone * b = BQ.back();
    BQ.pop_back();
    // Add to list
    A.push_back(a);
    B.push_back(b);
    // get children
    std::vector<Bone*> b_children = b->get_children();
    std::vector<Bone*> a_children = a->get_children();
    // a and b should have same number of children
    if(a_children.size() != b_children.size())
    {
      return false;
    }
    // Add children to queue
    AQ.insert(AQ.end(),a_children.begin(),a_children.end());
    BQ.insert(BQ.end(),b_children.begin(),b_children.end());
  }
  assert(AQ.size() == BQ.size());
  assert(A.size() == B.size());
  return true;
}
