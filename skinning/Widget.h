#ifndef WIDGET_H
#define WIDGET_H

// Generic widget class
//
// A widget is a class that can be drawn or clicked on
class Widget
{
  ///////////////////////////////////////////////////////////////////////////
  // Public fields
  ///////////////////////////////////////////////////////////////////////////
  public:
    // True if down has been called after last up
    bool is_down;
    bool is_selected;
    bool is_hover;
    // Mouse location at last down or drag call
    int last_x;
    int last_y;
    // Mouse location at down
    int down_x;
    int down_y;
  ///////////////////////////////////////////////////////////////////////////
  // Public functions
  ///////////////////////////////////////////////////////////////////////////
  public:
    Widget():
      is_down(false),
      is_selected(false),
      is_hover(false){}
    virtual ~Widget(){};
    // Called on each mouse DOWN event with mouse settings. Returns true if
    // caller should consider the mouse event "handled" by this widget. False
    // if caller should consider the mouse event un-"handled"
    //
    // Implementation is responsible for setting:
    //   is_down, is_selected, down_x, down_y, last_x, last_y 
    //
    // Inputs:
    //   x  x-position of mouse in screen space
    //   y  y-position of mouse in screen space
    //   right_click  true if mouse event was with right_click
    //   shift_down  true if shift modifier is down
    //   control_down  true if control modifier is down
    //   meta_down  true if meta modifier is down
    // Returns true if event is handled by this widget, false otherwise
    virtual bool down(
        int /*x*/, 
        int /*y*/, 
        bool /*right_click=false*/, 
        bool /*shift_down=false*/, 
        bool /*control_down=false*/, 
        bool /*meta_down=false*/)
    { return false;};
    // Called on each mouse UP event with mouse settings. Returns true if
    // caller should consider the mouse event "handled" by this widget. False
    // if caller should consider the mouse event un-"handled".
    //
    // Implementation is responsible for setting:
    //   is_down, is_selected, down_x, down_y, last_x, last_y 
    //
    // Inputs:
    //   x  x-position of mouse in screen space
    //   y  y-position of mouse in screen space
    //   right_click  true if mouse event was with right_click
    //   shift_down  true if shift modifier is down
    //   control_down  true if control modifier is down
    //   meta_down  true if meta modifier is down
    // Returns true if event is handled by this widget, false otherwise
    virtual bool up(
        int  /*x*/, 
        int  /*y*/, 
        bool /*right_click=false*/, 
        bool /*shift_down=false*/,
        bool /*control_down=false*/, 
        bool /*meta_down=false*/)
    {
      if(is_down){
        is_down = false;
        return true;
      }
      return false;
    };
    // Called on each mouse DRAG event with mouse settings. Returns true if
    // caller should consider the mouse event "handled" by this widget. False
    // if caller should consider the mouse event un-"handled".
    //
    // Implementation is responsible for setting:
    //   is_down, is_selected, down_x, down_y, last_x, last_y 
    //
    // Inputs:
    //   x  x-position of mouse in screen space
    //   y  y-position of mouse in screen space
    //   right_click  true if mouse event was with right_click
    //   shift_down  true if shift modifier is down
    //   control_down  true if control modifier is down
    //   meta_down  true if meta modifier is down
    // Returns true if event is handled by this widget, false otherwise
    virtual bool drag(
        int /*x*/, 
        int /*y*/, 
        bool /*right_click=false*/, 
        bool /*shift_down=false*/, 
        bool /*control_down=false*/, 
        bool /*meta_down=false*/)
    { return false;};
    // Called on each mouse MOUSE event with mouse settings. Returns true if
    // caller should consider the mouse event "handled" by this widget. False
    // if caller should consider the mouse event un-"handled".
    //
    // Implementation is responsible for setting:
    //   is_down, is_selected, down_x, down_y, last_x, last_y 
    //
    // Inputs:
    //   x  x-position of mouse in screen space
    //   y  y-position of mouse in screen space
    //   shift_down  true if shift modifier is down
    //   control_down  true if control modifier is down
    //   meta_down  true if meta modifier is down
    // Returns true if event is handled by this widget, false otherwise
    virtual bool move(
        int /*x*/, 
        int /*y*/, 
        bool /*shift_down=false*/, 
        bool /*control_down=false*/, 
        bool /*meta_down=false*/)
    { return false;};
    // Determines whether given screen space position is "inside" or "hitting"
    // this widget
    // Inputs:
    //   x  x-position of point in screen space
    //   y  y-position of point in screen space
    // Returns true if point is inside widget, false otherwise
    virtual bool inside(int /*x*/, int /*y*/){ return false;};
    // Draws the widget. Caller is responsible for setting the proper scene
    // matrices so the widget actually appears
    virtual void draw(){
      //std::cout<<"  Drawing a normal widget..."<<std::endl;
    };

};

#endif
