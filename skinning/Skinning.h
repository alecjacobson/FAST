//// Occasionally test this on and off to make sure passing Eigen matrices to
//// OpenGL works both ways, 
//#define EIGEN_DEFAULT_TO_ROW_MAJOR

//#define PBS_KAAN

// Forward declaration of ReAntTweakBar, wrapper for UI library AntTweakBar
namespace igl
{
  namespace anttweakbar
  {
    class ReTwBar;
  }
}

#ifndef NO_PUPPET
#  ifdef LOGPUPPET
#    include "LogPuppet.h"
#  else
#    include "PuppetInterface.h"
#  endif
#endif

#include <map>
#include <string>
#include <vector>

// To store matrices, lists of vectors
// To store index matrices, lists of vectors of indices
#include <Eigen/Core>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>

//typedef Eigen::SparseMatrix<double> LbsMatrixType;
typedef Eigen::MatrixXd LbsMatrixType;

#include "ShaderMode.h"
#include "Camera.h"
#include "ViewVector.h"
#include <igl/NormalType.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#elif defined(_WIN32)
#    define NOMINMAX
#    include <Windows.h>
#    undef NOMINMAX
#else
#define GL_GLEXT_PROTOTYPES
#  include <GL/gl.h>
#  include <GL/glext.h>
#endif

#include <igl/arap_dof.h>

#include "Skeleton.h"
//template <typename BoneType>
//class Skeleton;
#include "Skeleton.cpp"


#include "Animation.h"
#include "BoneBoneCopyMap.h"
#include "ExtraWeight.h"
#include "Ease.h"
#include "ColorOption.h"

#ifdef PBS_KAAN
    #include "collision_spheres.h"
    #include <AntTweakBar.h>
#endif

class Skinning
{
  ///////////////////////////////////////////////////////////////////////////
  // Public fields
  ///////////////////////////////////////////////////////////////////////////
  public:
    /////////////////////////////////////////////////////////////////////////
    // Mesh
    /////////////////////////////////////////////////////////////////////////
    // #V by 3 Matrix of mesh vertex 3D positions
    Eigen::MatrixXd V;
    // #V by 1 
    Eigen::MatrixXd V_depth_offsets;
    // #F by 3 Matrix of face (triangle) indices
    Eigen::MatrixXi F;
    // #Tets by 4 Matrix of tet (indices), empty means surface
    Eigen::MatrixXi Tets;
    // #V by 3 Matrix of per-mesh vertex 3D normal vectors
    Eigen::MatrixXd N;
    // #V by 3 Matrix of per-mesh vertex 3D normal vectors
    Eigen::MatrixXd CN;
    // #CN by 3 Matrix of per-mesh face 3D normal vectors
    Eigen::MatrixXd FN;
    // #V by 3 Matrix of per-mesh vertex 3D color vectors
    Eigen::MatrixXd C;
    // #F*3 by 3 Matrix of per-mesh vertex 3D region color vectors
    Eigen::MatrixXd RC;
    //// #V by 2 Matrix of mesh vertex texture coordinates
    //Eigen::MatrixXd TC;
    // #TC by 2 Matrix of mesh vertex texture coordinates
    Eigen::MatrixXd TC;
    // #F by 3 list of indices into texture coordinates TC, -1 means no texture
    Eigen::MatrixXi TF;
    // #F by 3 list of indices into normals CN, -1 means no texture
    Eigen::MatrixXi NF;
    // Original #V by #original_weights Matrix of per-mesh vertex, per-handle
    // weights unsorted.
    Eigen::MatrixXd OW;
    // Extra weights, #V by #extra_weights Matrix of per-mesh vertex,
    // per-handle weights (probably don't partition unity)
    Eigen::MatrixXd EW;
    // #V by
    // min(#original_weights+#extra_weights,MAX_NUM_WEIGHTS_PER_VERTEX)
    // Matrix of per-mesh vertex, per-handle weights sorted by value
    Eigen::MatrixXd W;
    // #V by
    // min(#original_weights+#extra_weights,MAX_NUM_WEIGHTS_PER_VERTEX)
    // Matrix of per-mesh vertex, per-handle indices corresponding to handles
    // of weights in W sorted by value
    Eigen::MatrixXi WI;
    // LBS matrix as being used by ARAP_DOF
    LbsMatrixType M;
    // used to store CPU version of deformed mesh vertices and normals
    Eigen::MatrixXd cpuV,cpuN;
    // use cpu to deform and recompute normals
    bool use_cpu;
    int old_bar_color[3];
    // "horizontally stacked" transformation matrices
    // If a single transformation is a #rows by #cols matrix then T is a
    // #rows by #actual_weights*#cols matrix
    Eigen::MatrixXf T;
    // Copy used by bypass_auto_dof
    Eigen::MatrixXf T_at_bypass;
    // Texture mapping ID
    GLuint texture_id;
    /////////////////////////////////////////////////////////////////////////
    // Handles
    /////////////////////////////////////////////////////////////////////////
    // Skeleton containing bone trees (roots)
    Skeleton<Bone> * skel;
    // Draw handles
    bool draw_bones;
    // Region colors from weights, etc.
    ColorOption color_option;
    // Use bone's last_T to *set* the transformation of the bone, i.e. don't
    // use bones' rotation/translation/etc to set T. Thus last_T should be
    // directly edited ("Dialed in") for each bone.
    bool dial_in_each_T;

    // Animation stack
    Animation<BoneBoneCopyMap> animation;
    // Animation start time
    double animation_start_seconds;
    // Currently animating
    bool animating;
    // Phony animation (this,rest,this)
    bool phony_animation;
    // number of seconds to interpolate this keyframe and the next one
    double animation_interp_secs;
    // Transition type to be used for next keyframe
    TransitionType transition_type;
    Animation<Camera> camera_animation;
    // Cheat x-translate
    double cheat_x;


    /////////////////////////////////////////////////////////////////////////
    // Auto DOF
    /////////////////////////////////////////////////////////////////////////
    bool auto_dof;
    bool bypass_auto_dof;
    int num_groups;
    int num_extra_weights;
    int max_iters;
    double tol;
    typedef float SSCALAR;
    igl::ArapDOFData<LbsMatrixType,SSCALAR> arap_dof;
    // Number of seconds that should pass in the dynamics simulation before
    // updating
    double seconds_per_update;
    double sort_weights_epsilon_power;
    // Current/Last frame's solution
    Eigen::MatrixXd L;
    // Extra weight type
    ExtraWeight ew_type;
    // Ease filter to use with certain extra weight methods
    Ease ease;
    // Push factor to use with certain extra weight methods
    double push;
    // use abrupt filter on weight space before computing extra weights
    int num_abrupt_weight_space;

    /////////////////////////////////////////////////////////////////////////
    // Display fields
    /////////////////////////////////////////////////////////////////////////
    int display_count;
    // seconds elapsed since epoch at start of fps counter
    double start_seconds;
    // Has Display() ever been called?
    bool virgin_display;
    // Was there any damage to the screen? AKA Do we need to redraw, redisplay
    bool damage;
    // Should we recompile the current display of the mesh
    bool display_list_damage;
    // OpenGL id for current display list
    GLuint display_list_id;
    // Use display list around opengl calls
    bool use_display_list;
    // Direction of point light in scene
    float light_direction[3], light2_direction[3];
    // light multiplier ~~ brightness of light, more like exposure
    float light_multiplier;
    float light2_multiplier;
    // Material properties
    float mat_ambient[4];
    float mat_diffuse[4];
    float mat_specular[4];
    float mat_shininess;
    float region_color[4];
    float lead_color[4];
#define NUM_CROWD_COLORS 4
    float crowd_colors[NUM_CROWD_COLORS][4];
    // display mesh with filled faces
    bool show_faces;
    // display mesh with solid lines
    bool show_lines;
    // Lock rotation
    bool scale_and_shift_on_load;
    // Scale
    double scale;
    // Shift
    double shift[3];
    // Threshold for snapping to closest canonical view quat
    double snap_threshold;
    // Min and max zoom constants
    const double min_zoom;
    const double max_zoom;
    // Allow zooming
    bool zoom_locked;
    Camera camera;
    // allow panning 
    bool pan_locked;
    // Size of opengl context, always updated immediately on resize
    double width;
    double height;
    // angle/perspective setting, opengl gluperspective angle, but also effects
    const double min_angle;
    // Allow angle/perspective change
    bool angle_locked;
    // Frames per second of last display
    double fps;
    // Milliseconds per fram
    double mspf;
    // number of frames per lap (before updating fps)
    int frames_per_lap;
    // Always clear the screen
    bool always_clear;
    // clear screen with time dependent color
    bool time_clear_color;
    // Background color
    float background[4];
    // use per vertex/corner/face normals
    igl::NormalType normal_type;
    // sharp corner threshold in degrees
    double corner_threshold;
    // Use texture mapping
    bool use_texture_mapping;
    // Reverse 2d depth offsets
    bool reverse_depth_offsets;
    // Save animation to TGA files
    bool render_animation_to_tga;
    // Save next frame to TGA file
    bool render_once;
    // Used for TGA filenames
    int render_count;

    /////////////////////////////////////////////////////////////////////////
    // Shaders
    /////////////////////////////////////////////////////////////////////////
    // Have shaders been loaded?
    bool shaders_loaded;
    // Mode of shader (see ShaderMode.h)
    ShaderMode shader_mode;
    // Shaders
    GLuint shader_programs[NUM_SHADER_MODE];
    // Maps vertex attribute names to attribute locations. See
    // glBindAttribLocation
    std::map<std::string,GLuint> shader_attribs[NUM_SHADER_MODE];
    // Selected weight function shown in scalar shader
    int selected_weight;
    
    /////////////////////////////////////////////////////////////////////////
    // UI fields
    /////////////////////////////////////////////////////////////////////////
    // UI object, wraps AntTweakBar library
    igl::anttweakbar::ReTwBar * rebar;
    // Whether to draw anttweakbar
    bool draw_anttweakbar;
    // Controls how fast the trackball feels, 1 is normal
    double trackball_speed_factor;
    // Keep track of whether mouse is down
    bool mouse_is_down;
    // AntTweakBar needs to know the current absolute mouse wheel (scroll)
    // position at all times
    float mouse_scroll_y;
    // Keep track of mouse position at mouse down
    int down_mouse_x;
    int down_mouse_y;
    // Keep track of whether click occurred in anttweakbar
    bool down_in_anttweakbar;
    // Keep track of whether trackball is ON
    bool trackball_on;
    // Keep track of whether panning is ON
    bool pan_on;
    // Keep track of pan at mouse down
    Camera down_camera;
    double down_scene_mouse[3];
    // Projected z-coordinate of mouse down
    double down_mouse_z;
    // Whether to rotate bone roots around origin or adjust translations to
    // rotate around current position
    bool center_root_rotations;
    /////////////////////////////////////////////////////////////////////////
    // Animation fields
    /////////////////////////////////////////////////////////////////////////
    // Controls whether we're animating a spin around the up axis
    bool spinning_about_up_axis;
    // Current time at start of spin
    double spin_start_seconds;
    // Keep track of rotation at start of spin
    double spin_start_rotation[4];
    // Seconds per full spin
    double spin_period;

    /////////////////////////////////////////////////////////////////////////
    // Crowd fields
    /////////////////////////////////////////////////////////////////////////
    bool reverse_lead;
    float lead_armadillo_distance;
    bool play_animation;
    bool display_crowd;
    // Last frame's solution for each crowd element + 1 for lead (last)
    Eigen::MatrixXd crowd_L;
    int crowd_x;
    int crowd_y;
    float crowd_x_off;
    float crowd_y_off;
    bool display_floor;
    float floor_depth;
    // global animation time increased by 33ms each frame (i.e. assuming 30FPS playback)
    float anim_timer;

#ifndef NO_PUPPET
    /////////////////////////////////////////////////////////////////////////
    // Puppet Fields
    /////////////////////////////////////////////////////////////////////////
#  ifdef LOG_PUPPET
    LogPuppet * log_puppet;
#  else
    PuppetInterface * pi;
#  endif
    bool using_puppet;
#endif

  ///////////////////////////////////////////////////////////////////////////
  // Public Functions
  ///////////////////////////////////////////////////////////////////////////
  public:
    Skinning();
    ~Skinning();
    ///////////////////////////////////////////////////////////////////////////
    // Session
    ///////////////////////////////////////////////////////////////////////////
    // Save session files to folder
    // Inputs:
    //   folder_name  name of folder to save current state. If folder does not
    //     already exist this try to create it 
    // Returns true on success and false on error
    bool save(const std::string folder_name);
    // load session from folder.
    // Inputs:
    //   folder_name  name of folder from which to load current state.
    // Returns true on success and false on error
    bool load(const std::string folder_name);
    ///////////////////////////////////////////////////////////////////////////
    // Display and Rendering
    ///////////////////////////////////////////////////////////////////////////
    // Initialize the skeleton
    void initialize_skeleton();
    // Initialize display
    // Set default values for all display related fields
    void initialize_display();
    // Initialize "UI" via (Re)AntTweakBar
    void initialize_anttweakbar();
    // Add anttweakbar variables related to info
    void add_info_anttweakbar_group();
    // Add anttweakbar variables related to 3D View
    void add_3d_view_anttweakbar_group();
    // Add anttweakbar bone editor group
    void add_bone_editor_group();
    // Add anttweakbar transformation editor group
    void add_transformation_editor_group();
    // Add anttweakbar auto dof group
    void add_auto_dof_group();
    // Add anttweakbar animation group
    void add_animation_group();
    // Add anttweakbar crowd group
    void add_crowd_group();
    // Add anttweakbar puppet group
    void add_puppet_group();
    // Set TweakBar to read only
    // Inputs:
    //   name  name of tw bar variable
    //   v  bool whether readonly or not
    void set_twbarvar_readonly(const char * name,const bool v);
    // Display the current mesh/scene
    void display();
    // Display a crowd of the current mesh and animation via OpenGL calls.
    // Should be called inbetween push/pop scene.
    void crowd();
    // Resize OpenGL scene
    // Inputs:
    //   width  new width of opengl display rectangle
    //   height  new height of opengl display rectangle
    void resize(const double width, const double height);
    // Resize anttweakbar, called whenever resize is called
    void resize_anttweakbar(const double width, const double height);
    // Push scene. Pan, zoom, rotate
    void push_scene();
    // Pop the current scene
    void pop_scene();
    // Set up material properties
    void material();
    // Arrange and turn on lights
    void lights();
    // Clear the screen
    void clear();
    // Sets colors of anttweakbar to give indication of Current state:
    // black --> release mode
    // debug --> debug mode
    // purple --> using cpu to deform
    // green --> editing bones 
    // yellow foreground --> bypassing autodof
    //
    void set_anttweakbar_colors();
    ///////////////////////////////////////////////////////////////////////////
    // Mouse/Keyboard interaction
    ///////////////////////////////////////////////////////////////////////////
    // Initialize Mouse and Keyboard
    void initialize_mouse_and_keyboard();
    // Handle key down event
    // Inputs:
    //   key  just pressed down key
    //   mouse_x  x-position of mouse
    //   mouse_y  y-position of mouse
    //   shift_down  whether shift modifier is down
    //   control_down  whether control modifier is down (on mac this translates
    //     to the command key)
    //   meta_down  whether meta modifier is down (on mac this translates to
    //     the "option" key)
    bool key_down(
      int key, 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle key up event
    bool key_up(
      int key, 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle (left) mouse down event
    bool mouse_down( 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle (left) mouse up event
    bool mouse_up( 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle (left) mouse move event
    bool mouse_move( 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle (left) mouse drag event
    bool mouse_drag( 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle right mouse down event
    bool right_mouse_down( 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle right mouse up event
    bool right_mouse_up( 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle right mouse drag event
    bool right_mouse_drag( 
      int mouse_x,
      int mouse_y,
      bool shift_down,
      bool control_down,
      bool meta_down);
    // Handle mouse scrolling
    // Inputs:
    //   ...
    //   delta_x  change in scroll x direction
    //   delta_y  change in scroll y direction
    bool mouse_scroll( int mouse_x, int mouse_y, float delta_x, float delta_y);
    ///////////////////////////////////////////////////////////////////////////
    // Mesh
    ///////////////////////////////////////////////////////////////////////////
    // Loads a mesh from a file into V,F
    // Inputs:
    //   mesh_file_name  path to mesh file (obj/off)
    // Returns true on success, false on errors
    bool load_mesh_from_file(const std::string mesh_file_name);
    // Called after loading V,F,Tets
    bool initialize_mesh();
    // Loads a mesh from a file into V,F
    // Inputs:
    //   mesh_file_name  path to mesh file (obj/off)
    // Returns true on success, false on errors
    bool save_deformed_mesh_to_file(const std::string mesh_file_name);
    // Load texture
    //   texture_name path to texture image file (tga)
    bool load_texture(const std::string tga_file_name);
    // Initializes texture mapping on the current mesh and texture
    bool initialize_texture_mapping();
    // Sets V(:,3) for planar meshes to depth offsets based on OW weights 
    // Returns true if changed V
    bool initialize_depth_offsets();
    ///////////////////////////////////////////////////////////////////////////
    // Rotation animation
    ///////////////////////////////////////////////////////////////////////////
    void initialize_rotation_animation();
    void update_rotation_animation();
    // Spin the current rotation about the "up" axis as a function of time
    // elapsed since begun spinning
    void spin_about_up_axis();
    ///////////////////////////////////////////////////////////////////////////
    // Shaders
    ///////////////////////////////////////////////////////////////////////////
    // Because glCreateShader needs a proper opengl context, this needs to be
    // called right before the first display
    void initialize_shaders();
    void deinitialize_shaders();
    // Validate state for current shader
    // Returns true if current shader is valid, returns false if current shader
    // is invalid
    bool validate_for_shader();
    // Loads shaders from the path prefix of shader_file_name and the .vert and
    // .frag extensions.
    //
    // Sets shader_*[LBS] values
    //
    // Inputs:
    //   idx  ShaderMode index (type of shader)
    //   shader_file_name  path to shader file, either .vert or .frag or prefix
    //     to these files
    // Returns true on success, false on errors
    bool load_shader_pair_from_files(
      const ShaderMode idx,
      const std::string shader_file_name);
    ///////////////////////////////////////////////////////////////////////////
    // LBS
    ///////////////////////////////////////////////////////////////////////////
    // Set up LBS transformations
    // Returns true on success, false on error
    bool transformations();
    // Send LBS transformations to shader
    bool send_transformations();
    // Load weights from a given .dmat file
    //
    // sets OW and reinitializes weights in shader
    //
    // Input:
    //  weights_file_name  path to .dmat matrix file containing weights
    // Returns true on success, false on error
    bool load_weights(const std::string weights_file_name);
    // Load extra weights from a given .dmat file
    //
    // sets EW and reinitializes weights in shader
    //
    // Input:
    //  weights_file_name  path to .dmat matrix file containing extra weights
    // Returns true on success, false on error
    bool load_extra_weights(const std::string weights_file_name);
    // Save extra weights to a given .dmat file
    //
    // Input:
    //  weights_file_name  path to .dmat matrix file to write extra weights
    // Returns true on success, false on error
    bool save_extra_weights(const std::string weights_file_name);
    // Initialize weights, sets up W,WI based on TW
    bool initialize_weights();
    // Initialize transformations for each handle to Identity
    bool initialize_transformations();
    // Load bone roots from a given .bf file
    //
    // sets skel->roots
    //
    // Input:
    //   bone_roots_file_name  path to .bf bone roots file
    //   must_match  whether bone forest must perfectly match current one
    //   no_recompute  don't recompute auto dof even if it's using it. Better match
    //     dof_types!! it's your funeral
    // Returns true on success, false on error
    bool load_bone_roots(
      const std::string bone_roots_file_name, 
      const bool must_match=false,
      const bool no_recompute=false);
    // Reads lbs.app's .tgf and .dmat pairs into BR and OW
    bool load_tgf_and_dmat_pair(
      const std::string tgf_file_name, 
      const std::string dmat_file_name);
    // Loads a bone roots animation for a given file and file's of a similar
    // name. The first pose is located in bone_roots_file_name which looks
    // something like /path/to/bone_roots.bf the subsequent poses are in files
    // which look like /path/to/bone_roots-1.bf, /path/to/bone_roots-2.bf and
    // so on
    //
    // Input:
    //   bone_roots_file_name  file name of first pose
    //
    bool load_bone_roots_animation(const std::string bone_roots_file_name);
    // Save bone roots to a given .bf file
    bool save_bone_roots(const std::string bone_roots_file_name);
    // Save bone roots animation to a given .bf file and subsequent files see
    // load_bone_roots_animation for naming convention
    // 
    // Input:
    //   bone_roots_file_name  file name of first pose
    bool save_bone_roots_animation(const std::string bone_roots_file_name);
    ///////////////////////////////////////////////////////////////////////////
    // Auto DOF
    ///////////////////////////////////////////////////////////////////////////
    // Returns a const reference to elements we're auto-doffing over. Either to
    // (triangles) F or (tetrahedra) Tets
    const Eigen::MatrixXi & elements();
    // Compute extra weights based on original weights, original mesh, and extra
    // weight type, sets EW
    //
    // Returns true on success, false otherwise
    bool compute_extra_weights();
    // Set default values for automatic degrees of freedom related variables
    // Returns true on success, false on error
    bool initialize_auto_dof();
    // Reinitialize auto dof without compleletly redoing precomputation, should
    // be called only when free/fixed change
    bool reinitialize_auto_dof();
    // Need to register a callback with the skeleton to know when the skelton
    // has gone into "edit mode" (either from the UI or because of the PUPPET)
    void after_skeleton_set_editing(const Skeleton<Bone> &);
    ///////////////////////////////////////////////////////////////////////////
    // Animation
    ///////////////////////////////////////////////////////////////////////////
    // Update animated elements
    // Return true only if animation has been changed
    bool update_animation(double t);
    // Start animation
    // Reture true if animation was really started
    bool start_animating();
    // Stopped animation
    // Reture true only if animation was stopped 
    bool stop_animating();
#ifndef NO_PUPPET
    ///////////////////////////////////////////////////////////////////////////
    // Puppet
    ///////////////////////////////////////////////////////////////////////////
    // Initialize the puppet and for now the LogPuppet to be read from a file
    bool initialize_puppet();
#endif
    
    void add_PBS_group();
#ifdef PBS_KAAN
    Spheres spheres;
    int sphere_levels;
    void sphere_constructor();
    
    bool draw_spheres;
    bool draw_deformed_spheres;
    
    void show_spheres();
    
    bool spheres_ready;
    bool collide_with_floor;
    bool naive_floor;
    SpheresVector SC;
    double collisions_K;
    double collisions_param;
    double collisions_p;
    bool water_instead_of_floor;
    float water_color[4];
    double water_size;
#endif
    
    double dampening;
};
