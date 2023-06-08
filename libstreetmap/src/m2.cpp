/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "gtk/gtk.h"

#include <iostream>
#include <string>
#include <sstream>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "dataStructures.h"
#include <chrono>
#include <fstream>
#include <math.h> 

#define MAX_ZOOM_IN   6e+4
#define ZOOM_LEVEL_1  1e+5
#define ZOOM_LEVEL_2  1e+6
#define ZOOM_LEVEL_3  1e+7
#define ZOOM_LEVEL_4  1e+8
#define ZOOM_LEVEL_5  5e+8
#define MAX_ZOOM_OUT  3.2e+9

// Move declarations into a header file:

///For draw Intersections
struct Intersection_Data {
   ezgl::point2d xy_loc = {0,0};
   std::string name;
   bool highlight = false;
};

std::vector<Intersection_Data> intersections; // load this vector in loadMap()
std::vector<IntersectionIdx> highlighted_intersections;

// Global Variable for selected x and y value:
double selected_x, selected_y;
bool selected = false;
bool highlighted = false;
std::string bearing;
std::pair<IntersectionIdx, IntersectionIdx> start_end;

void draw_main_canvas(ezgl::renderer *g);
void act_on_mouse_click(ezgl::application* application, GdkEventButton* /*event*/, double x, double y);
void search_box(GtkWidget* widget, ezgl::application* application);
void initial_setup(ezgl::application* application, bool /*new_window*/);
void search_bar_cbk(GtkWidget* /*entry_ptr*/, ezgl::application* application);
void controller_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application);
void saved_location_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application);
void friends_button_cbk (GtkWidget* /*widget_ptr*/, ezgl::application* application);
void friends_window_cbk (GtkWidget* /*widget_ptr*/, ezgl::application* application);
void save_location_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application);
void new_loc_cbk (GtkWidget* entry, ezgl::application* application);
void search_saved_entry_cbk(GtkWidget* /*entry_ptr*/, ezgl::application* application);
void open_loc_cbk(GtkWidget* /*window*/, ezgl::application* application);
void delete_saved_entry_cbk(GtkWidget* /*entry_ptr*/, ezgl::application* application);
void draw_street_segment(ezgl::renderer *g, double);
void draw_features(ezgl::renderer *g);
void draw_POIs(ezgl::renderer *g);
double lon_from_x(double x);
double lat_from_y(double y);
void draw_segment(ezgl::renderer *g, LatLon, LatLon, std::string,bool);
void save_new_location(LatLon new_location, std::string name, ezgl::application* application);
void draw_street_name(ezgl::renderer *g, StreetIdx, LatLon, LatLon, bool, bool);
void combo_box_cbk (GtkWidget* /*entry*/, ezgl::application* application);
void draw_intersections(ezgl::renderer *g);
void write_street_name(ezgl::renderer *g, StreetSegmentIdx id, bool highlight);
void directions_cbk (GtkWidget* /*widget_ptr*/, ezgl::application* application);
void help_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application);
void close_dialog(GtkDialog *dialog);
void draw_highlighted(std::vector<StreetSegmentIdx> path, ezgl::renderer *g);
void highlight_segment(ezgl::renderer *g, LatLon point_1, LatLon point_2);
void draw_arrow(ezgl::renderer *g, LatLon point_1, LatLon point_2);
std::string determineBearing(int x1, int y1, int x2, int y2);
void clear_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application);

std::string mapStatus;
//std::string getOSMWayTagValue(OSMID, std::string);
void drawHighway(ezgl::renderer *g, std::string, float, float, float, float);
void seg_info_calculation(ezgl::renderer *g, StreetSegmentIdx, bool);
std::vector<StreetSegmentIdx> segments_to_draw;
std::vector<StreetSegmentIdx> highlight_seg;
std::string left_or_right(std::string bearing1, std::string bearing2);
std::stringstream navigation(std::vector<StreetSegmentIdx> path, std::pair<IntersectionIdx, IntersectionIdx> start_end);

void drawMap() {
    // For draw intersections
    // Finding map bounds
    double max_x = x_from_lon(getIntersectionPosition(0).longitude());
    double min_x = max_x;
    double max_y = y_from_lat(getIntersectionPosition(0).latitude());
    double min_y = max_y;
    intersections.resize(getNumIntersections());
    
    for (int id = 0; id < getNumIntersections(); ++id) {
        intersections[id].xy_loc.x = x_from_lon(getIntersectionPosition(id).longitude());
        intersections[id].xy_loc.y = y_from_lat(getIntersectionPosition(id).latitude());
        intersections[id].name = getIntersectionName(id);
        
        max_x = std::max(max_x, intersections[id].xy_loc.x);
        min_x = std::min(min_x, intersections[id].xy_loc.x);
        max_y = std::max(max_y, intersections[id].xy_loc.y);
        min_y = std::min(min_y, intersections[id].xy_loc.y);
    }

    std::cout << max_x << std::endl;
    std::cout << min_x << std::endl;
    
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    ezgl::application application(settings);

    ezgl::rectangle initial_world({min_x, min_y}, {max_x, max_y});
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    // Uses GTK & CSS for customizing the interface:
    gtk_init(NULL, NULL);
    GtkCssProvider* css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "./libstreetmap/resources/style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    application.run(initial_setup, act_on_mouse_click, nullptr, nullptr);
}

// Draws the map on the canvas using EZGL:
void draw_main_canvas(ezgl::renderer *g) { 
      
    ezgl::rectangle world = g->get_visible_world();
    double area = world.area();
    
    g->draw_rectangle({0, 0},{1000, 1000});
    draw_features(g);

    draw_street_segment(g,area);

    if (world.area() < ZOOM_LEVEL_2) {
        draw_intersections(g);
    }
        
    draw_POIs(g);
        
    // Draw selected point:
    g->set_color(ezgl::RED);
    if (selected && world.area() < ZOOM_LEVEL_1) {
        g->fill_arc(ezgl::point2d{selected_x, selected_y}, 4, 0, 360);

    } else if (selected && world.area() < ZOOM_LEVEL_2) {
        g->fill_arc(ezgl::point2d{selected_x, selected_y}, 10, 0, 360);

    } else if (selected && world.area() < ZOOM_LEVEL_3) {
        g->fill_arc(ezgl::point2d{selected_x, selected_y}, 20, 0, 360);

    } else if (selected && world.area() < ZOOM_LEVEL_4) {
        g->fill_arc(ezgl::point2d{selected_x, selected_y}, 50, 0, 360);

    } else if (selected && world.area() > ZOOM_LEVEL_4) {
        g->fill_arc(ezgl::point2d{selected_x, selected_y}, 200, 0, 360);
    }

//    auto currTime = std::chrono::high_resolution_clock::now();
//    auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>>(currTime - startTime);
//    std::cout << "draw main canvas took" << wallClock.count() << "seconds" << std::endl;
    //std::cout << "area is: "<< world.area()<<std::endl;
    if(highlight_seg.size() != 0)
        draw_highlighted(highlight_seg,g);
    
    
}

double x_from_lon(double lon) {
    return kEarthRadiusInMeters*(lon*(M_PI/180))*cos(lat_avg);
}

double lon_from_x(double x) {
    return x / kEarthRadiusInMeters / cos(lat_avg) / (M_PI/180);
}

double y_from_lat(double lat) {
    return kEarthRadiusInMeters*(lat*(M_PI/180));
}

double lat_from_y(double y) {
    return y / kEarthRadiusInMeters / (M_PI/180);
}

void draw_POIs(ezgl::renderer *g) {
     ezgl::surface *restaurant = ezgl::renderer::load_png("libstreetmap/src/restaurant.png");
    //ezgl::surface *cafe = g->load_png();
    //ezgl::surface *fast_food = g->load_png();
    //ezgl::surface *ice_cream = g->load_png();
    //ezgl::surface *vending_machine = g->load_png();
    //ezgl::surface *bicycle_rental = g->load_png();
    //ezgl::surface *bus_station = g->load_png();
    //ezgl::surface *pharmacy = g->load_png();
    //ezgl::surface *doctors = g->load_png();
    //ezgl::surface *school = g->load_png();
    //ezgl::surface *place_of_worship = g->load_png();
    
    for (auto point : ListOfPOIsInType_restaurant) {
        //std::cout<< point.x << point.y << std::endl;
        //g->fill_rectangle(point, 100,100);
        if (g->get_visible_world().area() < ZOOM_LEVEL_2) {
            g->draw_surface(restaurant,point,125/(g->get_visible_world().top()- g->get_visible_world().bottom()));
        }    
    }
}

void draw_features(ezgl::renderer *g) {
    //std::vector< std::vector<ezgl::point2d> > ListOfFeaturePointsInFeaturePARK;
    g->set_color(222,221,226,225);
    //ezgl::renderer::get_visible_word().first is y bound
    //ezgl::renderer::get_visible_word().second is x bound
    ezgl::rectangle world = g->get_visible_world();
    g->fill_rectangle(world);
    
    double visible_bound_min_y = g->get_visible_world().bottom();
    double visible_bound_min_x = g->get_visible_world().left();
    double visible_bound_max_y = g->get_visible_world().top();
    double visible_bound_max_x = g->get_visible_world().right();
    
    ///LAKE///
    for (int i = 0; i < ListOfFeaturePointsInFeatureLAKE.size(); i++) {
        bool isVisible = true;

        if ((ListOfFeaturePointsInFeatureLAKE[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeatureLAKE[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeatureLAKE[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeatureLAKE[i].max_x < visible_bound_min_x)) {

            isVisible = false;
        }
            
        if ((ListOfFeaturePointsInFeatureLAKE[i].pointsInAFeature.size() > 1 ) && 
            (ListOfFeaturePointsInFeatureLAKE[i].closedPoly) && (isVisible) ) {

            //Draw features
            g->set_color(109,182,187,255);
            g->fill_poly(ListOfFeaturePointsInFeatureLAKE[i].pointsInAFeature);  
        }
    } 
    
    ///ISLAND///
    for (int i = 0; i < ListOfFeaturePointsInFeatureISLAND.size(); i++) {
        bool isVisible = true;

        if ((ListOfFeaturePointsInFeatureISLAND[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeatureISLAND[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeatureISLAND[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeatureISLAND[i].max_x < visible_bound_min_x)) {

            isVisible = false;
        }
            
        if (( ListOfFeaturePointsInFeatureISLAND[i].pointsInAFeature.size() > 1 ) &&
            (ListOfFeaturePointsInFeatureISLAND[i].closedPoly) && (isVisible) ) {
        
            g->set_color(236,210,137,255);
            g->fill_poly(ListOfFeaturePointsInFeatureISLAND[i].pointsInAFeature);
            
            //Draw feature names          
        //    std::string Name = ListOfFeaturePointsInFeatureISLAND[i].FeatureName;
        //    g -> set_font_size(10);
        //    g -> set_color(0,0,0); 
        //    double mid_x = 0.5 * (ListOfFeaturePointsInFeatureISLAND[i].min_x + ListOfFeaturePointsInFeatureISLAND[i].max_x);
        //    double mid_y = 0.5 * (ListOfFeaturePointsInFeatureISLAND[i].min_y + ListOfFeaturePointsInFeatureISLAND[i].max_y);
        //    if (Name != "<noname>") {
        //        g -> draw_text({mid_x, mid_y}, Name);
        //    }
        }
        
    }
    
    ///PARK///
    for (int i = 0; i < ListOfFeaturePointsInFeaturePARK.size(); i++) {
        bool isVisible = true;

        if ((ListOfFeaturePointsInFeaturePARK[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeaturePARK[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeaturePARK[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeaturePARK[i].max_x < visible_bound_min_x)) {

            isVisible = false;
        }
            
        if ((ListOfFeaturePointsInFeaturePARK[i].pointsInAFeature.size() > 1 ) &&
            (ListOfFeaturePointsInFeaturePARK[i].closedPoly) && (isVisible) ) {
                
            g->set_color(88,140,127,255);
            g->fill_poly(ListOfFeaturePointsInFeaturePARK[i].pointsInAFeature);
            
            //Draw feature names
//            std::string Name = ListOfFeaturePointsInFeaturePARK[i].FeatureName;
//            g -> set_font_size(10);
//            g -> set_color(0,0,0); 
//            double mid_x = 0.5 * (ListOfFeaturePointsInFeaturePARK[i].min_x + ListOfFeaturePointsInFeaturePARK[i].max_x);
//            double mid_y = 0.5 * (ListOfFeaturePointsInFeaturePARK[i].min_y + ListOfFeaturePointsInFeaturePARK[i].max_y);
//            if (Name != "<noname>") {
//                g -> draw_text({mid_x, mid_y}, Name);
//            }  
        } 
    }

    ///GREENSPACE///   
    for (int i = 0; i < ListOfFeaturePointsInFeatureGREENSPACE.size(); i++) {
        bool isVisible = true;

        if ((ListOfFeaturePointsInFeatureGREENSPACE[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeatureGREENSPACE[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeatureGREENSPACE[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeatureGREENSPACE[i].max_x < visible_bound_min_x)) {

            isVisible = false;
        }
            
        if (( ListOfFeaturePointsInFeatureGREENSPACE[i].pointsInAFeature.size() > 1 ) && 
            (ListOfFeaturePointsInFeatureGREENSPACE[i].closedPoly) && (isVisible) && 
            (world.area() < 500000000)) {

            g->set_color(172,188,139,255);
            g->fill_poly(ListOfFeaturePointsInFeatureGREENSPACE[i].pointsInAFeature);
            
            //Draw feature names
//            std::string Name = ListOfFeaturePointsInFeatureGREENSPACE[i].FeatureName;
//            g -> set_font_size(10);
//            g -> set_color(0,0,0); 
//            double mid_x = 0.5 * (ListOfFeaturePointsInFeatureGREENSPACE[i].min_x + ListOfFeaturePointsInFeatureGREENSPACE[i].max_x);
//            double mid_y = 0.5 * (ListOfFeaturePointsInFeatureGREENSPACE[i].min_y + ListOfFeaturePointsInFeatureGREENSPACE[i].max_y);
//            if(Name != "<noname>")
//            {
//                g -> draw_text({mid_x, mid_y}, Name);
//            }
                
        }
    }

    ///BEACH///    
    for (int i = 0; i < ListOfFeaturePointsInFeatureBEACH.size(); i++) {
        bool isVisible = true;

        if ((ListOfFeaturePointsInFeatureBEACH[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeatureBEACH[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeatureBEACH[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeatureBEACH[i].max_x < visible_bound_min_x)) {

            isVisible = false;
        }
            
        if (( ListOfFeaturePointsInFeatureBEACH[i].pointsInAFeature.size() > 1 ) &&
            (ListOfFeaturePointsInFeatureBEACH[i].closedPoly) && (isVisible) && (world.area() < 800000000)) {

            g->set_color(241,179,119,255);
            g->fill_poly(ListOfFeaturePointsInFeatureBEACH[i].pointsInAFeature);
        }  
    }
    
    ///GLACIER///
    for (int i = 0; i < ListOfFeaturePointsInFeatureGLACIER.size(); i++) {
        bool isVisible = true;

        if ((ListOfFeaturePointsInFeatureGLACIER[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeatureGLACIER[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeatureGLACIER[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeatureGLACIER[i].max_x < visible_bound_min_x)) {

            isVisible = false;
        }
            
        if (( ListOfFeaturePointsInFeatureGLACIER[i].pointsInAFeature.size() > 1 ) &&
            (ListOfFeaturePointsInFeatureGLACIER[i].closedPoly) && (isVisible) ) {

            g->set_color(185,189,200,255);
            g->fill_poly(ListOfFeaturePointsInFeatureGLACIER[i].pointsInAFeature);
        }
    } 
    
    ///GOLFCOURSE///
    for (int i = 0; i < ListOfFeaturePointsInFeatureGOLFCOURSE.size(); i++) {
        bool isVisible = true;

        if ((ListOfFeaturePointsInFeatureGOLFCOURSE[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeatureGOLFCOURSE[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeatureGOLFCOURSE[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeatureGOLFCOURSE[i].max_x < visible_bound_min_x)) {

            isVisible = false;
        }
            
        if (( ListOfFeaturePointsInFeatureGOLFCOURSE[i].pointsInAFeature.size() > 1 ) &&
            (ListOfFeaturePointsInFeatureGOLFCOURSE[i].closedPoly) && (isVisible) && (world.area() < 500000000)) {

            g->set_color(132,164,140,255);
            g->fill_poly(ListOfFeaturePointsInFeatureGOLFCOURSE[i].pointsInAFeature);
        } 
    } 
    
    ///RIVER///
    g->set_color(1,124,164,255);
    g->set_line_width(5);
    
    for (int i = 0; i < ListOfFeaturePointsInFeatureRIVER.size(); i++) {
        bool isVisible = true;

        if ((ListOfFeaturePointsInFeatureRIVER[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeatureRIVER[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeatureRIVER[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeatureRIVER[i].max_x < visible_bound_min_x)) {

            isVisible = false;
        }
        
        if (( ListOfFeaturePointsInFeatureRIVER[i].pointsInAFeature.size() > 1 ) &&
            (ListOfFeaturePointsInFeatureRIVER[i].closedPoly) && (isVisible) && (world.area() < 500000000)) {

           
            g->fill_poly(ListOfFeaturePointsInFeatureRIVER[i].pointsInAFeature);

        } else if ((g->get_visible_world().contains(ListOfFeaturePointsInFeatureRIVER[i].pointsInAFeature[0])) &&
            (isVisible) && (world.area() < 500000000)) {

            for (int j = 0; j < ((ListOfFeaturePointsInFeatureRIVER[i].pointsInAFeature.size())-1); j++) {                    
                g->draw_line(ListOfFeaturePointsInFeatureRIVER[i].pointsInAFeature[j], 
                ListOfFeaturePointsInFeatureRIVER[i].pointsInAFeature[j+1]);
            }
        }
    }

    ///STREAM///
    g->set_color(137,193,210,255);
    g->set_line_width(20000/(world.top()- world.bottom()));
    
    for (int i = 0; i < ListOfFeaturePointsInFeatureSTREAM.size(); i++) {
        //bool isVisible = true;

        if ((ListOfFeaturePointsInFeatureSTREAM[i].min_y > visible_bound_max_y) ||
            (ListOfFeaturePointsInFeatureSTREAM[i].max_y < visible_bound_min_y) ||
            (ListOfFeaturePointsInFeatureSTREAM[i].min_x > visible_bound_max_x) ||
            (ListOfFeaturePointsInFeatureSTREAM[i].max_x < visible_bound_min_x)) {

            //isVisible = false;
        }
        
       
        for (int j = 0; j < ((ListOfFeaturePointsInFeatureSTREAM[i].pointsInAFeature.size())-1); j++) {                    
            g->draw_line(ListOfFeaturePointsInFeatureSTREAM[i].pointsInAFeature[j], 
            ListOfFeaturePointsInFeatureSTREAM[i].pointsInAFeature[j+1]);
        }
    }
 
    ///BUILDINGS///
    if (world.area() < ZOOM_LEVEL_2) {
        for (int i = 0; i < ListOfFeaturePointsInFeatureBUILDING.size(); i++) {
                bool isVisible = true;

                if ((ListOfFeaturePointsInFeatureBUILDING[i].min_y > visible_bound_max_y) ||
                    (ListOfFeaturePointsInFeatureBUILDING[i].max_y < visible_bound_min_y) ||
                    (ListOfFeaturePointsInFeatureBUILDING[i].min_x > visible_bound_max_x) ||
                    (ListOfFeaturePointsInFeatureBUILDING[i].max_x < visible_bound_min_x)) {

                    isVisible = false;
                }
                    
                if (( ListOfFeaturePointsInFeatureBUILDING[i].pointsInAFeature.size() > 1) &&
                    (ListOfFeaturePointsInFeatureBUILDING[i].closedPoly) && (isVisible) &&
                    (world.area() < ZOOM_LEVEL_3)) {

                    g->set_color(208,148,122,255);
                    g->fill_poly(ListOfFeaturePointsInFeatureBUILDING[i].pointsInAFeature);
                    
                    //Draw feature names
        //            std::string Name = ListOfFeaturePointsInFeatureBUILDING[i].FeatureName;
        //            g -> set_font_size(10);
        //            g -> set_color(0,0,0); 
        //            double mid_x = 0.5 * (ListOfFeaturePointsInFeatureBUILDING[i].min_x + ListOfFeaturePointsInFeatureBUILDING[i].max_x);
        //            double mid_y = 0.5 * (ListOfFeaturePointsInFeatureBUILDING[i].min_y + ListOfFeaturePointsInFeatureBUILDING[i].max_y);
        //            if(Name != "<noname>")
        //            {
        //                g -> draw_text({mid_x, mid_y}, Name);
        //            }
            }
        }    
    }
}

  ///Draw Intersection
void draw_intersections(ezgl::renderer *g) {
    
    for (size_t intersectionNum = 0; intersectionNum < intersections.size(); ++intersectionNum) {
       if (intersections[intersectionNum].highlight) {
           g->set_color(158,102,94,255);
       } else {
           g->set_color(219,107,93,100);
       }

      g->fill_elliptic_arc((intersections[intersectionNum].xy_loc),5,5,0,360);
    }
}



void draw_segment(ezgl::renderer *g, LatLon point_1, LatLon point_2, std::string way_type, bool highlight) {
    float x1, x2, y1, y2;
    int width1;

    x1 = x_from_lon(point_1.longitude());
    x2 = x_from_lon(point_2.longitude());

    y1 = y_from_lat(point_1.latitude());
    y2 = y_from_lat(point_2.latitude());
    
    if (way_type == "motorway" || way_type == "secondary") {
        drawHighway(g, way_type,x1,x2,y1,y2);
    }

    ezgl::rectangle world = g->get_visible_world();

    if (world.area() < ZOOM_LEVEL_2) {
        if(highlight)
            g->set_color(255,0,0);
        else
            g->set_color(255,255,255);
            
        g->set_line_width(5000/(world.top()-world.bottom()));
        g->draw_line({x1,y1},{x2,y2});
             
    } else if ((world.area() < ZOOM_LEVEL_3)) {
        width1 = 3;
        if(highlight)
            g->set_color(255,0,0);
        else
            g->set_color(255,255,255);
        g->set_line_width(width1);
        g->draw_line({x1,y1},{x2,y2});
    }
}

void drawHighway(ezgl::renderer *g, std::string way_type, float x1, float x2, float y1, float y2){
    
    if (way_type == "motorway") {
        g->set_color(255,255,255); 
        g->set_line_width(10);
        g->draw_line({x1,y1},{x2,y2});
        
        g->set_color(255,102,178); 
        g->set_line_width(5);
        g->draw_line({x1,y1},{x2,y2});


    } else if (way_type == "secondary") {
        g->set_color(255,255,255); 
        g->set_line_width(10);
        g->draw_line({x1,y1},{x2,y2});

        g->set_color(236,210,137); 
        g->set_line_width(5);
        g->draw_line({x1,y1},{x2,y2});   
        
    } else if (way_type == "primary") {
        g->set_color(225,25,18); 
        g->set_line_width(10);
        g->draw_line({x1,y1},{x2,y2});

    } else if (way_type == "trunk") {
        g->set_color(246,250,187); 
        g->set_line_width(10);
        g->draw_line({x1,y1},{x2,y2});

    } else if (way_type == "tertiary") {
        g->set_color(246,250,187); 
        g->set_line_width(10);
        g->draw_line({x1,y1},{x2,y2});

    } else if (way_type == "unclassified") {
        g->set_color(246,250,187); 
        g->set_line_width(10);
        g->draw_line({x1,y1},{x2,y2});

    } else if (way_type == "residential") {
        g->set_color(246,250,187); 
        g->set_line_width(10);
        g->draw_line({x1,y1},{x2,y2});
    }

}

void draw_street_name(ezgl::renderer *g, StreetIdx street_id, LatLon point_1, LatLon point_2, bool oneway, bool highlight) {
    float x1, x2, y1, y2, x_mid, y_mid;
    double PI = 3.14159265;

    x1 = x_from_lon(point_1.longitude());
    x2 = x_from_lon(point_2.longitude());
    x_mid = (x1+x2)/2;

    y1 = y_from_lat(point_1.latitude());
    y2 = y_from_lat(point_2.latitude());
    y_mid = (y1+y2)/2;

    float degrees;
    if (x2 == x1) {

        if (y2 > y1) {
            degrees = 90;
        } else {
            degrees = -90;  
        }  

    } else {
        degrees = atan((y2-y1)/(x2-x1))* 180 / PI;
    }
    
    ezgl::rectangle world = g->get_visible_world();
    
    if (world.area() < ZOOM_LEVEL_2) {
            if (world.contains({x1,y1}) || world.contains({x1,y1})) {
                g->set_text_rotation(degrees);
                std::string StreetName = getStreetName(street_id);

                if(oneway) {
                    StreetName = StreetName.append("->");
                }
                    
                g->set_font_size(5000/(world.top()-world.bottom()));
                
                if(highlight)
                    g->set_color(255,255,255);
                else
                    g->set_color(0,0,0); 
                g->format_font(MapAndFont[mapStatus], ezgl::font_slant::normal, ezgl::font_weight::normal);

                if ((StreetName != "<unknown>")&&(StreetName != "<unknown>->")) {
                    g->draw_text({x_mid,y_mid}, StreetName);
                }  
                     
            }
                
            
    } 
}

void seg_info_calculation(ezgl::renderer *g, StreetSegmentIdx id, bool highlight){   
    LatLon point_1, point_2, curve_ponit_1, curve_ponit_2;

    float x1, x2, y1, y2, max_x, max_y, min_x, min_y;
    
    StreetSegmentInfo Street_Seg_Info = getStreetSegmentInfo(id);
    point_1 = getIntersectionPosition(Street_Seg_Info.from);
    point_2 = getIntersectionPosition(Street_Seg_Info.to);
    x1 = x_from_lon(point_1.longitude());
    x2 = x_from_lon(point_2.longitude());
    y1 = y_from_lat(point_1.latitude());
    y2 = y_from_lat(point_2.latitude());
    
    double visible_bound_min_y = g->get_visible_world().bottom();
    double visible_bound_min_x = g->get_visible_world().left();
    double visible_bound_max_y = g->get_visible_world().top();
    double visible_bound_max_x = g->get_visible_world().right();

    if(x1 > x2){
        max_x = x1;
        min_x = x2;   
    }
    else{
        max_x = x2;
        min_x = x1;
    }
        
    
    if(y1 > y2){
        max_y = y1;
        min_y = y2;   
    }
    else{
        max_y = y2;
        min_y = y1;
    }
    
    if(max_x < visible_bound_min_x || min_x > visible_bound_max_x 
       || max_y < visible_bound_min_y || min_y > visible_bound_max_y){  
        return;
    }
    
    segments_to_draw.push_back(id);
    OSMID way_OSMID = Street_Seg_Info.wayOSMID;
    std::string way_type = getOSMWayTagValue(way_OSMID, "highway");

    if (Street_Seg_Info.numCurvePoints == 0) {
        // if the street segment is a straight line:      
        draw_segment(g, point_1, point_2, way_type, highlight);
       
    } 
    else {  // if not a straight line, split up the street segment into small straight lines

        // and add all of the lengths up
        int curve_total = Street_Seg_Info.numCurvePoints;

        if (curve_total == 1) {
            curve_ponit_1 = getStreetSegmentCurvePoint(id, 0);
            draw_segment(g, point_1, curve_ponit_1, way_type, highlight); 
            draw_segment(g, curve_ponit_1, point_2, way_type, highlight);    
        } 

        else {
            //curved streets
            for (int curve_Point_Idx = 0; curve_Point_Idx <= curve_total-2; curve_Point_Idx++) {
                curve_ponit_1 = getStreetSegmentCurvePoint(id, curve_Point_Idx);
                curve_ponit_2 = getStreetSegmentCurvePoint(id, curve_Point_Idx+1);                 
                draw_segment(g, curve_ponit_1, curve_ponit_2, way_type, highlight);

            }

            curve_ponit_1 = getStreetSegmentCurvePoint(id, 0);
            draw_segment(g, point_1, curve_ponit_1, way_type, highlight);

            curve_ponit_2 = getStreetSegmentCurvePoint(id, curve_total-1);
            draw_segment(g, curve_ponit_2, point_2, way_type, highlight);    
        }
    }          
}

void  write_street_name(ezgl::renderer *g, StreetSegmentIdx id, bool highlight){
        LatLon point_1, point_2, curve_ponit_1, curve_ponit_2;
        StreetSegmentInfo Street_Seg_Info = getStreetSegmentInfo(id);
        point_1 = getIntersectionPosition(Street_Seg_Info.from);
        point_2 = getIntersectionPosition(Street_Seg_Info.to);
        StreetIdx street_id = Street_Seg_Info.streetID;
        std::string street_name = getStreetName(street_id);
        int num_of_chars = street_name.length();
        bool oneway = Street_Seg_Info.oneWay;
        double length = findStreetSegmentLength(id);
      
        //if (Street_Seg_Info.numCurvePoints == 0) {
        // if the street segment is a straight line:      
            if(length > 6*num_of_chars){
                draw_street_name(g, street_id, point_1,point_2,oneway,highlight);
                
            }
            //std::cout<<"the length is:"<<length<<" and the num of chars is "<<num_of_chars<<" and the street is: "<<street_name<<std::endl;   

        //}  
    
}


void draw_street_segment(ezgl::renderer *g, double area) {

    std::vector<IntersectionIdx>::iterator it;
    std::vector<StreetSegmentIdx> vec1;
    segments_to_draw.clear();

    
    if(area > ZOOM_LEVEL_4){
        //draw motorways
        for(int j = 0; j < motorway_segments.size(); j++){
            seg_info_calculation(g,motorway_segments[j],highlighted);           
        }
        vec1 = segments_to_draw;
        std::sort(vec1.begin(),vec1.end());
        it = std::unique(vec1.begin(),vec1.end()); 

        // Resizing the vector so as to remove the undefined terms
        vec1.resize(std::distance(vec1.begin(), it));
        segments_to_draw = vec1;   

        for(int i = 0; i < segments_to_draw.size(); i++){
            write_street_name(g, segments_to_draw[i],highlighted);
        }     
    }
   
    else if(area < ZOOM_LEVEL_4 && area > ZOOM_LEVEL_3){
        //draw motorways
        for(int j = 0; j < motorway_segments.size(); j++){
            seg_info_calculation(g,motorway_segments[j],highlighted);           
        }
        for(int i = 0; i < secondary_way_segments.size(); i++){
            seg_info_calculation(g,secondary_way_segments[i],highlighted);           
        }
        
        vec1 = segments_to_draw;
        std::sort(vec1.begin(),vec1.end());
        it = std::unique(vec1.begin(),vec1.end()); 

        // Resizing the vector so as to remove the undefined terms
        vec1.resize(std::distance(vec1.begin(), it));
        segments_to_draw = vec1;   

        for(int i = 0; i < segments_to_draw.size(); i++){
            write_street_name(g, segments_to_draw[i], highlighted);
        }       
        
    }
    else{
        for(int j = 0; j < motorway_segments.size(); j++){
            seg_info_calculation(g,motorway_segments[j],highlighted);           
        }
        for(int i = 0; i < secondary_way_segments.size(); i++){
            seg_info_calculation(g,secondary_way_segments[i],highlighted);           
        }
        for(int k = 0; k < remaining_segments.size(); k++){
            seg_info_calculation(g,remaining_segments[k],highlighted);
        }
        vec1 = segments_to_draw;
        std::sort(vec1.begin(),vec1.end());
        it = std::unique(vec1.begin(),vec1.end()); 

        // Resizing the vector so as to remove the undefined terms
        vec1.resize(std::distance(vec1.begin(), it));
        segments_to_draw = vec1;   

        for(int i = 0; i < segments_to_draw.size(); i++){
            write_street_name(g, segments_to_draw[i],highlighted);
        }
    }
}


// Used to define GTK widgets on the UI:
void initial_setup(ezgl::application* application, bool /*new_window*/) {
    // Create autocomplete for search bar:
    GtkListStore* autocomplete = GTK_LIST_STORE(application->get_object("StreetNames"));

    for (std::map<std::string, int>::iterator it = fullStreetNames.begin(); it != fullStreetNames.end(); ++it) {
        GtkTreeIter iter;

        GValue* name = g_new0(GValue, 1);
        gchar* val = strdup((it->first).c_str());
        g_value_init(name, G_TYPE_STRING);
        
        g_value_take_string(name, val);

        gtk_list_store_append(autocomplete, &iter);

        gtk_list_store_set_value(autocomplete, &iter, 0, name);
        g_free(name);
        free(val);
    }

    // Search Bar Definition:
    GObject* search_bar_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    search_bar_button = application->get_object("SearchButton");
    g_signal_connect(search_bar_button, "clicked", G_CALLBACK(search_bar_cbk), application);

    GObject* search_bar_1 = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    search_bar_1 = application->get_object("SearchBar");
    g_signal_connect(search_bar_1, "activate", G_CALLBACK(search_bar_cbk), application);

    GObject* search_bar_2 = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    search_bar_2 = application->get_object("SearchBar2");
    g_signal_connect(search_bar_2, "activate", G_CALLBACK(search_bar_cbk), application);

    // Controller Toggle Button Definition:
    GObject* controller_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    controller_button = application->get_object("Controller");
    g_signal_connect(controller_button, "toggled", G_CALLBACK(controller_button_cbk), application);

    // Saved Location Button Definition:
    GObject* saved_location_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    saved_location_button = application->get_object("SavedLocationsButton");
    g_signal_connect(saved_location_button, "clicked", G_CALLBACK(saved_location_button_cbk), application);
    
    // Friends Button Definition:
    GObject* friends_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    friends_button = application->get_object("FriendsButton");
    g_signal_connect(friends_button, "clicked", G_CALLBACK(friends_button_cbk), application);

    // Friends Window Definition:
    GObject* friends_window = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    friends_window = application->get_object("FriendsWindow");
    g_signal_connect(friends_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), application);

    // Saved Location Window Definition:
    GObject* saved_location_window = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    saved_location_window = application->get_object("SavedLocations");
    g_signal_connect(saved_location_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), application);

    // Save Location Button Definition:
    GObject* save_location_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    save_location_button = application->get_object("SaveButton");
    g_signal_connect(save_location_button, "clicked", G_CALLBACK(saved_location_button_cbk), application);

    // Save Location Name Definition:
    GObject* save_location_name = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    save_location_name = application->get_object("LocationName");
    g_signal_connect(save_location_name, "activate", G_CALLBACK(save_location_cbk), application);

    // Combo Box Definition:
    GObject* combo_box = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    combo_box = application->get_object("SelectMap");
    g_signal_connect(combo_box, "changed", G_CALLBACK(combo_box_cbk), application);

    // Search Saved Location Definition:
    GObject* search_saved_location_entry = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    search_saved_location_entry = application->get_object("SearchSavedLocation");
    g_signal_connect(search_saved_location_entry, "activate", G_CALLBACK(search_saved_entry_cbk), application);

    // Search Saved Location Button Definition:
    GObject* search_saved_location_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    search_saved_location_button = application->get_object("SearchLocationButton");
    g_signal_connect(search_saved_location_button, "clicked", G_CALLBACK(search_saved_entry_cbk), application);

    // Delete Saved Location Button Definition:
    GObject* delete_saved_location_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    delete_saved_location_button = application->get_object("DeleteLocationButton");
    g_signal_connect(delete_saved_location_button, "activate", G_CALLBACK(delete_saved_entry_cbk), application);

    // Directions Button Definition:
    GObject* directions_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    directions_button = application->get_object("Directions");
    g_signal_connect(directions_button, "clicked", G_CALLBACK(directions_cbk), application);

    // Directions Window Definition:
    GObject* directions_window = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    directions_window = application->get_object("DirectionsWindow");
    g_signal_connect(directions_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), application);
    
    // Help Button Definition:
    GObject* help_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    help_button = application->get_object("HelpButton");
    g_signal_connect(help_button, "clicked", G_CALLBACK(help_button_cbk), application);

    // Clear Button Definition:
    GObject* clear_button = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
    clear_button = application->get_object("ClearButton");
    g_signal_connect(clear_button, "clicked", G_CALLBACK(clear_button_cbk), application);
    
    // help Window Definition:
//    GObject* help_window = (GObject*)g_object_new(G_TYPE_OBJECT, NULL);
//    help_window = application->get_object("DirectionsWindow");
//    g_signal_connect(help_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), application);

}

// Allows canvas to be clickable:
// Used for highlighting intersections:
void act_on_mouse_click(ezgl::application* app, GdkEventButton* /*event*/, double x, double y) {
    std::cout << "Mouse clicked at (" << x << "," << y << ")\n";

    LatLon position = LatLon(lat_from_y(y), lon_from_x(x));
    int inter_id = findClosestIntersection(position);
    
  

    // Check amount of click error (How far away it was to the intersection):
    //get the LatLon of the closest intersection
    LatLon intersection_position = getIntersectionPosition(inter_id);

    // Error = ((actual - clicked)/ actual)*100
    double error_x = (std::fabs(x_from_lon(intersection_position.longitude()) - x_from_lon(position.longitude()))) / (std::fabs(x_from_lon(intersection_position.longitude())));
    error_x = error_x*100.0;

    double error_y = (std::fabs(y_from_lat(intersection_position.latitude()) - y_from_lat(position.latitude()))) / (std::fabs(y_from_lat(intersection_position.latitude())));
    error_y = error_y*100.0;

    double area = ((app->get_renderer())->get_visible_world()).area();
    if (area < ZOOM_LEVEL_2 && error_x < 0.0001 && error_y < 0.0001) {
        if (intersections[inter_id].highlight == true) { // If it has been highlighted previously, unhighlight:
            intersections[inter_id].highlight = false;
            highlighted_intersections.erase(std::find(highlighted_intersections.begin(), highlighted_intersections.end(), inter_id));
            

            std::stringstream ss;
            ss << "Intersection Unselected: " << intersections[inter_id].name << "\n";
            app->update_message(ss.str());
            app->refresh_drawing();

        } else { // Highlight intersection clicked:
            intersections[inter_id].highlight = true;
            highlighted_intersections.push_back(inter_id);
            std::stringstream ss;
            ss << "Intersection Selected: " << intersections[inter_id].name << "\n";
            app->update_message(ss.str());
            app->refresh_drawing();
        }
        
    } else if (!selected) { // If the user did not intend to click an intersection:
        selected_x = x;
        selected_y = y;

        selected = true;

        std::stringstream ss;
        ss << "Point Selected." << "\n";
        app->update_message(ss.str());

        app->refresh_drawing();

        GtkRevealer* selected_popup = GTK_REVEALER(app->get_object("SelectedPopup"));
        GtkLabel* selected_label = GTK_LABEL(app->get_object("SelectedLocation"));

        std::stringstream location;
        location << "Latitude: " << position.latitude() << "   " << "Longitude: " << position.longitude();

        gtk_label_set_markup(selected_label, (location.str()).c_str());
        gtk_revealer_set_reveal_child(selected_popup, true);

    } else { // Unselect point:
        selected = false;
                  
        std::stringstream ss;
        ss << "Point Unselected." << "\n";
        app->update_message(ss.str());

        app->refresh_drawing();

        GtkRevealer* selected_popup = GTK_REVEALER(app->get_object("SelectedPopup"));
        gtk_revealer_set_reveal_child(selected_popup, false);
    }
    
}

// Callback function for the search bar widget (GtkEntry widget):
void search_bar_cbk(GtkWidget* /*entry_ptr*/, ezgl::application* application) {
    // Get user inputed text:
    GtkEntry* text_entry_1 = GTK_ENTRY(application->get_object("SearchBar"));
    const gchar* street_prefix_1 = gtk_entry_get_text(text_entry_1);

    GtkEntry* text_entry_2 = GTK_ENTRY(application->get_object("SearchBar2"));
    const gchar* street_prefix_2 = gtk_entry_get_text(text_entry_2);

    // If user does not input in both searchboxes:
    if ((char)(*street_prefix_1) == '\0' && (char)(*street_prefix_2) == '\0') {
        application->update_message("Please input 2 streets.");
        return;
    } else if ((char)(*street_prefix_1) == '\0') {
        application->update_message("Please input Street 1.");
        return;
    } else if ((char)(*street_prefix_2) == '\0') {
        application->update_message("Please input Street 2.");
        return;
    }

    // Use street_prefix to find related streets:
    std::vector<StreetIdx> street_ids_1 = findStreetIdsFromPartialStreetName(street_prefix_1);
    std::vector<StreetIdx> street_ids_2 = findStreetIdsFromPartialStreetName(street_prefix_2);

    // Check if streets exist:
    if (street_ids_1.empty() && street_ids_2.empty()) {
        application->update_message("Streets not found.");
        return;
    } else if (street_ids_1.empty()) {
        application->update_message("Street 1 not found.");
        return;
    } else if (street_ids_2.empty()) {
        application->update_message("Street 2 not found.");
        return;
    }

    // For now, only works if you correctly input the street name:
    std::vector<IntersectionIdx> found_intersections, temp_intersections; 
    int count = 0;

    // Find intersections between given street names:
    for (int street_count_1 = 0; street_count_1 < street_ids_1.size(); street_count_1++) {
        for (int street_count_2 = 0; street_count_2 < street_ids_2.size(); street_count_2++) {
            temp_intersections = findIntersectionsOfTwoStreets(street_ids_1[street_count_1], street_ids_2[street_count_2]);

            // Look if there are numerous roads with intersections:
            if (!temp_intersections.empty()) {
                found_intersections = temp_intersections;
                count++;
            }
        }
    }

    // Check if the vector is empty:
    if (found_intersections.empty()) {
        application->update_message("No intersections found.");
        return;
    } else if (count > 1) {
        application->update_message("Ambiguous streets given, please try again.");
        return;
    }

    // Highlight intersection(s) found:
    for (int counter = 0; counter < found_intersections.size(); counter++) {
        intersections[found_intersections[counter]].highlight = true;
        highlighted_intersections.push_back(found_intersections[counter]);
        std::stringstream ss;
        ss << "Intersection Selected: " << intersections[found_intersections[counter]].name << "\n";
        application->update_message(ss.str());
    }

    application->refresh_drawing();
}

// Callback function for the controller toggle button:
void controller_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application) {
    GtkToggleButton* controller_toggle = GTK_TOGGLE_BUTTON(application->get_object("Controller"));
    bool toggle = (bool)gtk_toggle_button_get_active(controller_toggle);

    GtkWidget* controller = GTK_WIDGET(application->get_object("ControllerGrid"));
    
    if (toggle) {
        gtk_widget_show(controller);
    } else {
        gtk_widget_hide(controller);
    }
}

void saved_location_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application) {
    GtkWidget* saved_locations_window = GTK_WIDGET(application->get_object("SavedLocations"));
    gtk_widget_show_all(saved_locations_window);

    GtkLabel* selected_label = GTK_LABEL(application->get_object("SavedLocationsList"));

    std::stringstream location;

    for (std::map<std::string,LatLon>::iterator it = saved_locations.begin(); it != saved_locations.end(); ++it) {
        location << "Name: " << it->first << "\n";
        location << "Latitude: " << (it->second).latitude() << "   " << "Longitude: " << (it->second).longitude() << "\n\n";
    }

    gtk_label_set_markup(selected_label, (location.str()).c_str());
}

void friends_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application) {
    GtkWidget* friends_window = GTK_WIDGET(application->get_object("FriendsWindow"));
    gtk_widget_show(friends_window);
    
    GtkWidget* friends_grid = GTK_WIDGET(application->get_object("FriendsGrid"));
    gtk_widget_show(friends_grid);
}

void save_new_location(LatLon new_location, std::string name, ezgl::application* application) {
    if ( auto it{ saved_locations.find(name) }; it != std::end(saved_locations)){
        application->update_message("Location name already taken.");
        return;
    }

    saved_locations[name] = new_location;
    application-> update_message("Location saved.");
    
    return;
}

void save_location_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application) {
   // Get user inputed text:
    GtkEntry* text_entry = GTK_ENTRY(application->get_object("LocationName"));
    const gchar* location_name = gtk_entry_get_text(text_entry);

    // If user does not input in box:
    if ((char)(*location_name) == '\0') {
        application->update_message("Please input a location name.");
        return;
    }

    if (!selected) {
        application->update_message("Please select a point.");
        return;
    }

    LatLon location(lat_from_y(selected_y), lon_from_x(selected_x));
    save_new_location(location, location_name, application);

    GtkLabel* selected_label = GTK_LABEL(application->get_object("SavedLocationsList"));

    std::stringstream location_list;

    for (std::map<std::string,LatLon>::iterator it = saved_locations.begin(); it != saved_locations.end(); ++it) {
        location_list << "Name: " << it->first << "\n";
        location_list << "Latitude: " << (it->second).latitude() << "   " << "Longitude: " << (it->second).longitude() << "\n\n";
    }

    gtk_label_set_markup(selected_label, (location_list.str()).c_str());

    gtk_entry_set_text(text_entry, "");
} 

void combo_box_cbk (GtkWidget* /*entry*/, ezgl::application* application) {
   // Get user inputed text:
    GtkComboBox* selection = GTK_COMBO_BOX(application->get_object("SelectMap"));
    int location = gtk_combo_box_get_active(selection);

    std::string map_path;

    if (location == 0) {
        map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
        mapStatus = "The rest";
    } else if (location == 1) {
        map_path = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
        mapStatus = "The rest";
    } else if (location == 2) {
        map_path = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
        mapStatus = "Cairo or Tehran";
    } else if (location == 3) {
        map_path = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
        mapStatus = "The rest";
    } else if (location == 4) {
        map_path = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
        mapStatus = "The rest";
    } else if (location == 5) {
        map_path = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
        mapStatus = "The rest";
    } else if (location == 6) {
        map_path = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
        mapStatus = "The rest";
    } else if (location == 7) {
        map_path = "/cad2/ece297s/public/maps/iceland.streets.bin";
        mapStatus = "The rest";
    } else if (location == 8) {
        map_path = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
        mapStatus = "The rest";
    } else if (location == 9) {
        map_path = "/cad2/ece297s/public/maps/kyiv_ukraine.streets.bin";
        mapStatus = "Kyiv";
    } else if (location == 10) {
        map_path = "/cad2/ece297s/public/maps/london_england.streets.bin";
        mapStatus = "The rest";
    } else if (location == 11) {
        map_path = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
        mapStatus = "The rest";
    } else if (location == 12) {
        map_path = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
        mapStatus = "The rest";
    } else if (location == 13) {
        map_path = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
        mapStatus = "The rest";
    } else if (location == 14) {
        map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
        mapStatus = "The rest";
    } else if (location == 15) {
        map_path = "/cad2/ece297s/public/maps/singapore.streets.bin";
        mapStatus = "The rest";
    } else if (location == 16) {
        map_path = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
        mapStatus = "The rest";
    } else if (location == 17) {
        map_path = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
        mapStatus = "Cairo or Tehran";
    } else if (location == 18) {
        map_path = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
        mapStatus = "The rest";
    } 

    closeMap();
    loadMap(map_path);
    intersections.clear();

    double max_x = x_from_lon(getIntersectionPosition(0).longitude());
    double min_x = max_x;
    double max_y = y_from_lat(getIntersectionPosition(0).latitude());
    double min_y = max_y;
    intersections.resize(getNumIntersections());
    
    for (int id = 0; id < getNumIntersections(); ++id) {
        intersections[id].xy_loc.x = x_from_lon(getIntersectionPosition(id).longitude());
        intersections[id].xy_loc.y = y_from_lat(getIntersectionPosition(id).latitude());
        intersections[id].name = getIntersectionName(id);
        
        max_x = std::max(max_x, intersections[id].xy_loc.x);
        min_x = std::min(min_x, intersections[id].xy_loc.x);
        max_y = std::max(max_y, intersections[id].xy_loc.y);
        min_y = std::min(min_y, intersections[id].xy_loc.y);
    }
    
    ezgl::rectangle initial_world({min_x, min_y}, {max_x, max_y});
    application->change_canvas_world_coordinates("MainCanvas", initial_world);
    draw_main_canvas(application->get_renderer());
    application->refresh_drawing();
}

// Callback function for the search saved location widget (GtkEntry widget):
void search_saved_entry_cbk(GtkWidget* /*entry_ptr*/, ezgl::application* application) {
    // Get user inputed text:
    GtkEntry* text_entry = GTK_ENTRY(application->get_object("SearchSavedLocation"));
    const gchar* location_name = gtk_entry_get_text(text_entry);

    // If user does not input in both searchboxes:
    if ((char)(*location_name) == '\0') {
        application->update_message("Please input one of the saved locations.");
        return;
    }

    std::string search_saved_location(location_name);
    LatLon location = saved_locations[search_saved_location];
    
    selected_x = x_from_lon(location.longitude());
    selected_y = y_from_lat(location.latitude());

    selected = true;

    std::stringstream ss;
    ss << "Point Selected." << "\n";
    application->update_message(ss.str());

    application->refresh_drawing();

    GtkRevealer* selected_popup = GTK_REVEALER(application->get_object("SelectedPopup"));
    GtkLabel* selected_label = GTK_LABEL(application->get_object("SelectedLocation"));

    std::stringstream update;
    update << "Latitude: " << location.latitude() << "   " << "Longitude: " << location.longitude();

    gtk_label_set_markup(selected_label, (update.str()).c_str());
    gtk_revealer_set_reveal_child(selected_popup, true);
    gtk_entry_set_text(text_entry, "");
}

// Callback function for the search saved location widget (GtkEntry widget):
void delete_saved_entry_cbk(GtkWidget* /*entry_ptr*/, ezgl::application* application) {
    // Get user inputed text:
    GtkEntry* text_entry = GTK_ENTRY(application->get_object("DeleteLocationButton"));
    const gchar* location_name = gtk_entry_get_text(text_entry);

    // If user does not input in both searchboxes:
    if ((char)(*location_name) == '\0') {
        application->update_message("Please input one of the saved locations.");
        return;
    }

    std::string search_saved_location(location_name);
    saved_locations.erase(search_saved_location);

    GtkLabel* selected_label = GTK_LABEL(application->get_object("SavedLocationsList"));

    std::stringstream location_list;

    for (std::map<std::string,LatLon>::iterator it = saved_locations.begin(); it != saved_locations.end(); ++it) {
        location_list << "Name: " << it->first << "\n";
        location_list << "Latitude: " << (it->second).latitude() << "   " << "Longitude: " << (it->second).longitude() << "\n\n";
    }

    gtk_label_set_markup(selected_label, (location_list.str()).c_str());

    gtk_entry_set_text(text_entry, "");
    application->update_message("Deleted location.");
}

void directions_cbk (GtkWidget* /*widget_ptr*/, ezgl::application* application) {
    std::stringstream directions;
    if(highlight_seg.size() != 0){
        highlight_seg.clear();
        application->refresh_drawing();
        return;
    }    
    // Get highlighted intersections:
    
    int size = highlighted_intersections.size();
    
    if(size != 2){
         application->update_message("Please highlight 2 locations.");
         return; 
    }

    start_end.first = highlighted_intersections[0];
    start_end.second = highlighted_intersections[1];

    // Find shortest path:
    std::vector<StreetSegmentIdx> path = findPathBetweenIntersections(start_end, 15);
    highlight_seg = path;
    directions = navigation(path,start_end);

    // Print shortest path:
    GtkWidget* directions_window = GTK_WIDGET(application->get_object("DirectionsWindow"));
    gtk_widget_show_all(directions_window);

    GtkLabel* label = GTK_LABEL(application->get_object("DirectionsLabel"));

    // Add words to stringstream directions

    gtk_label_set_markup(label, (directions.str()).c_str());
    application->refresh_drawing();
}



std::string left_or_right(std::string bearing1, std::string bearing2){
    std::string str;
    if(bearing1 == "east" ) {
        if(bearing2 == "northeast" || bearing2 == "north" || bearing2 == "northwest")
            str = "Turn left";
        else if(bearing2 == "east")
            str = "Keep going straight in this direction";
        else if(bearing2 == "west")
            str = "Turn back";
        else
            str = "Turn right";
    }
    
    else if(bearing1 == "west" ) {
        if(bearing2 == "northeast" || bearing2 == "north" || bearing2 == "northwest")
            str = "Turn right";
        else if(bearing2 == "west")
            str = "Keep going straight in this direction";
        else if(bearing2 == "east")
            str = "Turn back";
        else 
            str = "Turn left";
    }
    
    else if(bearing1 == "north" ) {
        if(bearing2 == "northeast" || bearing2 == "east" || bearing2 == "southeast")
            str = "Turn right";
        else if(bearing2 == "north")
            str = "Keep going straight in this direction";
        else if(bearing2 == "south")
            str = "Turn back";
        else
            str = "Turn left";
    }
    
    else if(bearing1 == "south" ) {
        if(bearing2 == "northeast" || bearing2 == "east" || bearing2 == "southeast")
            str = "Turn left";
        else if(bearing2 == "south")
            str = "Keep going straight in this direction";
        else if(bearing2 == "north")
            str = "Turn back";
        else
            str = "Turn right";
        
    }
    else if(bearing1 == "northeast" ) {
        if(bearing2 == "south" || bearing2 == "east" || bearing2 == "southeast")
            str = "Turn right";
        else if(bearing2 == "northeast")
            str = "Keep going straight in this direction";
        else
            str = "Turn left";
        
    } 
    
    else if(bearing1 == "northwest" ) {
        if(bearing2 == "northeast" || bearing2 == "east" || bearing2 == "north")
            str = "Turn right";
        else if(bearing2 == "northwest")
            str = "Keep going straight in this direction";
        else
            str = "Turn left";
    }
    
     else if(bearing1 == "southeast" ) {
        if(bearing2 == "northeast" || bearing2 == "east" || bearing2 == "north")
            str = "Turn left";
        else if(bearing2 == "southeast")
            str = "Keep going straight in this direction";
        else
            str = "Turn right";
            
    }
    
     else if(bearing1 == "southwest" ) {
        if(bearing2 == "south" || bearing2 == "east" || bearing2 == "southeast")
            str = "Turn left";
        else if(bearing2 == "southwest")
            str = "Keep going straight in this direction";
        else
            str = "Turn right";
    }

    
    
    return str;
}

void highlight_segment(ezgl::renderer *g, LatLon point_1, LatLon point_2){
    float x1, x2, y1, y2;
    x1 = x_from_lon(point_1.longitude());
    x2 = x_from_lon(point_2.longitude());
    y1 = y_from_lat(point_1.latitude());
    y2 = y_from_lat(point_2.latitude());
    ezgl::rectangle world = g->get_visible_world();

    g->set_color(255,0,0);
    g->set_line_width(5000/(world.top()-world.bottom()));
    g->draw_line({x1,y1},{x2,y2});

    
}
void draw_arrow(ezgl::renderer *g, LatLon point_1, LatLon point_2){
    float x1, x2, y1, y2;
    double PI = 3.14159265;
    x1 = x_from_lon(point_1.longitude());
    x2 = x_from_lon(point_2.longitude());
    y1 = y_from_lat(point_1.latitude());
    y2 = y_from_lat(point_2.latitude());
    double x_mid = (x1+x2)/2;
    double y_mid = (y1+y2)/2;
    float angle_in_rad;
    float angle_in_deg;


    if (x2 == x1) {
        if (y2 > y1) {
            angle_in_deg = 90;
        } else {
            angle_in_deg = -90;  
        }  

    } else {
        
        angle_in_rad = std::atan2(y2 - y1, x2 - x1);
        angle_in_deg = angle_in_rad * 180.0 / PI;
    }
    std::cout<<angle_in_deg<<std::endl;
    
    ezgl::rectangle world = g->get_visible_world();
    g->set_text_rotation(angle_in_deg);
    g->set_color(0,0,255);
    g->set_font_size(5000/(world.top()-world.bottom()));
    g->draw_text({x_mid,y_mid}, ">");
}


void  draw_highlighted(std::vector<StreetSegmentIdx> path, ezgl::renderer *g){
    StreetSegmentInfo first_seg_info, second_seg_info;
    int path_size = path.size();
    //std::cout<<path_size<<std::endl;
    


    //std::cout<<path_size<<std::endl;
    for(int i = 0; i < path.size(); i++){
        seg_info_calculation(g,path[i],true);      
    }
    
    LatLon point_1, point_2, point_3;


    if(path_size == 1){
        StreetSegmentInfo seg_info = getStreetSegmentInfo(path[0]);
        double length = findStreetSegmentLength(path[0]);
        StreetIdx street_id = seg_info.streetID;
        std::string street_name = getStreetName(street_id);
        IntersectionIdx seg_from, seg_to;
        seg_from = start_end.first;
        seg_to = start_end.second;
        point_1 = getIntersectionPosition(seg_from);
        point_2 = getIntersectionPosition(seg_to);
        draw_arrow(g,point_1, point_2);
       

        return;
    }
    
    
     
    for(int i = 0; i < path_size-1; i++){
        first_seg_info = getStreetSegmentInfo(path[i]);
        second_seg_info = getStreetSegmentInfo(path[i+1]);
        
        StreetIdx street_id1 = first_seg_info.streetID;
        std::string street_name1 = getStreetName(street_id1);
        StreetIdx street_id2 = second_seg_info.streetID;
        std::string street_name2 = getStreetName(street_id2);
        
        int seg1CurvePointsNum = first_seg_info.numCurvePoints;
        int seg2CurvePointsNum = second_seg_info.numCurvePoints;
        
        IntersectionIdx first_seg_from, first_seg_to, second_seg_to;
        if(first_seg_info.from == second_seg_info.from){
            if(!first_seg_info.oneWay){
                first_seg_from = first_seg_info.to; 
                first_seg_to = first_seg_info.from;
                second_seg_to = second_seg_info.to;
            }
        }
        else if(first_seg_info.from == second_seg_info.to){
            if(!first_seg_info.oneWay && !second_seg_info.oneWay ){
                first_seg_from = first_seg_info.to; 
                first_seg_to = first_seg_info.from;
                second_seg_to = second_seg_info.from;
            }
        }
        else if(first_seg_info.to == second_seg_info.from){
                first_seg_from = first_seg_info.from; 
                first_seg_to = first_seg_info.to;
                second_seg_to = second_seg_info.to;
        }
        else if(first_seg_info.to == second_seg_info.to){
            if(!second_seg_info.oneWay){
                first_seg_from = first_seg_info.from; 
                first_seg_to = first_seg_info.to;
                second_seg_to = second_seg_info.from;
            }
        }
            

        //straight - straight
        if (seg1CurvePointsNum == 0 && seg2CurvePointsNum == 0) {
            point_1 = getIntersectionPosition(first_seg_from);
            point_2 = getIntersectionPosition(first_seg_to);
            point_3 = getIntersectionPosition(second_seg_to);
        }
        //curve - straight
        else if(seg1CurvePointsNum != 0 && seg2CurvePointsNum == 0){
            point_1 = getStreetSegmentCurvePoint(path[i], seg1CurvePointsNum-1);
            point_2 = getIntersectionPosition(first_seg_to);
            point_3 = getIntersectionPosition(second_seg_to);
 
        }
        //straight - curve
        else if(seg1CurvePointsNum == 0 && seg2CurvePointsNum != 0){
            point_1 = getIntersectionPosition(first_seg_from);
            point_2 = getIntersectionPosition(first_seg_to);
            point_3 = getStreetSegmentCurvePoint(path[i+1], 0);
            
        }
        //curve - curve
        else{
            point_1 = getStreetSegmentCurvePoint(path[i], seg1CurvePointsNum-1);
            point_2 = getIntersectionPosition(first_seg_to);
            point_3 = getStreetSegmentCurvePoint(path[i+1], 0);
 
        }
        
        if(path_size >= 2 && i == path_size-2)
             draw_arrow(g,point_2, point_3);
            
            
        
             
        draw_arrow(g,point_1, point_2);
    }   
}

std::string determineBearing(int x1, int y1, int x2, int y2){
    if(x2 > x1 && y2 > y1)
        bearing = "northeast";
    else if(x2 > x1 && y2 < y1)
        bearing = "southeast";
    else if(x2 < x1 && y2 > y1)
        bearing = "northwest";
    else if(x2 < x1 && y2 < y1)
        bearing = "southwest";
    else if(x2 == x1 && y2 < y1)
        bearing = "south";
    else if(x2 == x1 && y2 > y1)
        bearing = "north";
    else if(x2 > x1 && y2 == y1)
        bearing = "east";
    else if(x2 < x1 && y2 == y1)
        bearing = "west";
    return bearing;
    
}

std::stringstream navigation(std::vector<StreetSegmentIdx> path, std::pair<IntersectionIdx, IntersectionIdx> start_end){
    LatLon point_1, point_2, point_3;
    double x1,y1,x2,y2,x3,y3;
    std::stringstream ss;
    int path_size = path.size();

    if(path_size == 1){
        StreetSegmentInfo seg_info = getStreetSegmentInfo(path[0]);
        double length = findStreetSegmentLength(path[0]);
        StreetIdx street_id = seg_info.streetID;
        std::string street_name = getStreetName(street_id);

        if (street_name == "<unknown>") {
            street_name = "unknown";
        }

        IntersectionIdx seg_from, seg_to;
        seg_from = start_end.first;
        seg_to = start_end.second;
        point_1 = getIntersectionPosition(seg_from);
        point_2 = getIntersectionPosition(seg_to);
        x1 = x_from_lon(point_1.longitude());
        y1 = y_from_lat(point_1.latitude());
        x2 = x_from_lon(point_2.longitude());
        y2 = y_from_lat(point_2.latitude());
        std::string bearing = determineBearing(x1,y1,x2,y2);
        ss <<"0. Starting from "<<street_name<<" head "<<bearing<<" ("<<length<<" meters)"<<"\n ";
        std::cout<<"0. Starting from "<<street_name<<" head "<<bearing<<" ("<<length<<" meters)"<<"\n ";
        return ss;
    }
        
    for(int i = 0; i < path_size-1; i++){
        StreetSegmentInfo first_seg_info = getStreetSegmentInfo(path[i]);
        StreetSegmentInfo second_seg_info = getStreetSegmentInfo(path[i+1]);
        
        StreetIdx street_id1 = first_seg_info.streetID;
        std::string street_name1 = getStreetName(street_id1);
        StreetIdx street_id2 = second_seg_info.streetID;
        std::string street_name2 = getStreetName(street_id2);

        if (street_name1 == "<unknown>") {
            street_name1 = "unknown";
        }

        if (street_name2 == "<unknown>") {
            street_name2 = "unknown";
        }
        
        double length1 = findStreetSegmentLength(path[i]);
        double length2 = findStreetSegmentLength(path[i+1]);
        
        int seg1CurvePointsNum = first_seg_info.numCurvePoints;
        int seg2CurvePointsNum = second_seg_info.numCurvePoints;
        
        IntersectionIdx first_seg_from, first_seg_to, second_seg_to;
        if(first_seg_info.from == second_seg_info.from){
            if(!first_seg_info.oneWay){
                first_seg_from = first_seg_info.to; 
                first_seg_to = first_seg_info.from;
                second_seg_to = second_seg_info.to;
            }
        }
        else if(first_seg_info.from == second_seg_info.to){
            if(!first_seg_info.oneWay && !second_seg_info.oneWay ){
                first_seg_from = first_seg_info.to; 
                first_seg_to = first_seg_info.from;
                second_seg_to = second_seg_info.from;
            }
        }
        else if(first_seg_info.to == second_seg_info.from){
                first_seg_from = first_seg_info.from; 
                first_seg_to = first_seg_info.to;
                second_seg_to = second_seg_info.to;
        }
        else if(first_seg_info.to == second_seg_info.to){
            if(!second_seg_info.oneWay){
                first_seg_from = first_seg_info.from; 
                first_seg_to = first_seg_info.to;
                second_seg_to = second_seg_info.from;
            }
        }
            

        //straight - straight
        if (seg1CurvePointsNum == 0 && seg2CurvePointsNum == 0) {
            point_1 = getIntersectionPosition(first_seg_from);
            point_2 = getIntersectionPosition(first_seg_to);
            point_3 = getIntersectionPosition(second_seg_to);
        }
        //curve - straight
        else if(seg1CurvePointsNum != 0 && seg2CurvePointsNum == 0){
            point_1 = getStreetSegmentCurvePoint(path[i], seg1CurvePointsNum-1);
            point_2 = getIntersectionPosition(first_seg_to);
            point_3 = getIntersectionPosition(second_seg_to);
 
        }
        //straight - curve
        else if(seg1CurvePointsNum == 0 && seg2CurvePointsNum != 0){
            point_1 = getIntersectionPosition(first_seg_from);
            point_2 = getIntersectionPosition(first_seg_to);
            point_3 = getStreetSegmentCurvePoint(path[i+1], 0);
            
        }
        //curve - curve
        else{
            point_1 = getStreetSegmentCurvePoint(path[i], seg1CurvePointsNum-1);
            point_2 = getIntersectionPosition(first_seg_to);
            point_3 = getStreetSegmentCurvePoint(path[i+1], 0);
 
        }
        x1 = x_from_lon(point_1.longitude());
        y1 = y_from_lat(point_1.latitude());
        x2 = x_from_lon(point_2.longitude());
        y2 = y_from_lat(point_2.latitude());
        x3 = x_from_lon(point_3.longitude());
        y3 = y_from_lat(point_3.latitude());
        std::cout<<"("<<x1<<","<<y1<<")"<<"("<<x2<<","<<y2<<")"<<"("<<x3<<","<<y3<<")"<<std::endl;
        
        
        
        std::string bearing1 = determineBearing(x1,y1,x2,y2);
        std::string bearing2 = determineBearing(x2,y2,x3,y3);
        double gradient = (y2-y1)/(x2-x1);
        double y_2_prime = (x3-x1)*gradient + y1;
        std::string direction = left_or_right(bearing1,bearing2);
        //std::cout<<i<<"."<<"bearing1: "<<bearing1<<" bearing2: "<<bearing2<<std::endl;
        if(bearing1 == "northeast" && bearing2 == "southwest"){
            if(y3 < y_2_prime)
                direction = "Turn right";
            else
                direction = "Turn left";  
        }
        else if(bearing1 == "northwest" && bearing2 == "southeast"){
            if(y3 < y_2_prime)
                direction = "Turn left";
            else
                direction = "Turn right";  
        }
        else if(bearing1 == "southeast" && bearing2 == "northwest"){
            if(y3 < y_2_prime)
                direction = "Turn right";
            else
                direction = "Turn left";  
        }
        else if(bearing1 == "southwest" && bearing2 == "northeast"){
            if(y3 < y_2_prime)
                direction = "Turn left";
            else
                direction = "Turn right";  
        }
        

        if(i == 0){
            ss<< i <<". Starting from "<<street_name1<<" head "<<bearing1<<" ("<<length1<<" meters)"<<"\n ";
            std::cout<< i <<". Starting from "<<street_name1<<" head "<<bearing1<<" ("<<length1<<" meters)"<<"\n ";
            if(path.size() == 1)
                return ss;
            ss<< direction <<", head "<<bearing2<<" onto "<<street_name2<<"("<<length2<<" meters)"<<"\n";   
            std::cout<< direction <<", head "<<bearing2<<" onto "<<street_name2<<"("<<length2<<" meters)"<<"\n"; 
        }
        else{
           
            ss<<i<<". "<<direction<<", head "<<bearing2<<" onto "<<street_name2<<"("<<length2<<" meters)"<<"\n"; 
            std::cout<<i<<". "<<direction<<", head "<<bearing2<<" onto "<<street_name2<<"("<<length2<<" meters)"<<"\n"; 
            if(i == path.size()-2)
                ss<<"Reaches the destination"<<"\n";
        }
       
    }
    std::string printout = ss.str();
    std::cout << printout << std::endl;
    

    return ss;
    
}

void close_dialog(GtkDialog *dialog){
     gtk_widget_destroy(GTK_WIDGET (dialog));
}

void help_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application) {
    GObject* window = application->get_object(application -> get_main_window_id().c_str());   
    GtkWidget* content_area; 
    GtkWidget* script;
    GtkWidget* dialog_window; 
    dialog_window = gtk_dialog_new_with_buttons("Instructions",(GtkWindow*) window, GTK_DIALOG_MODAL, ("Close"),GTK_RESPONSE_REJECT,NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_window));
    std::string str = "Hey! Welcome to the Manta Maps!!!        \n";
    str.append("Here are some instructions about how to use the map:     \n\n");
    str.append("1. The menu is located at the top-left corner of the window. You will see multiple buttons there.   \n");
    str.append("*you can change the city map at the drop-down list, save friend's location, use the controller   \n");
    str.append("to control the map, and exit the program.   \n\n");
    str.append("2. You can find the intersection of two streets by entering their names, valid intersection would   \n");
    str.append("be highlighted.   \n\n");
    str.append("3. Repeating step 2 twice and click Direction button, you will see the highlighted shortest path\n");
    str.append("between these 2 intersections.   \n\n");
    str.append("Enjoy the exploration!   \n");
    
    const gchar* c = str.c_str();
    script = gtk_label_new(c);
    gtk_container_add(GTK_CONTAINER(content_area), script);
   
    gtk_widget_show_all(dialog_window);
    g_signal_connect(GTK_DIALOG(dialog_window),"response",G_CALLBACK(close_dialog),NULL);

    return;
    
}

void clear_button_cbk(GtkWidget* /*widget_ptr*/, ezgl::application* application) {
    highlighted_intersections.clear();

    for (int i = 0; i < getNumIntersections(); i++) {
        intersections[i].highlight = false;
    }

    application->refresh_drawing();
}