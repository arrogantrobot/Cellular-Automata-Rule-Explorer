#include <unistd.h>
#include <gnome.h>
#include "CellularAutomaton.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <sys/types.h>
#include <dirent.h>

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define WIDTH_LOWER 400
#define WIDTH_UPPER 2048


#define ROW_LOWER_BOUND 1
#define ROW_UPPER_BOUND 255

#define LOOPTIME 30 //in milliseconds

#define ICON_16 "./data/CA_icon_16.png"
#define ICON_32 "./data/CA_icon_32.png"
#define ICON_48 "./data/CA_icon_48.png"
#define ICON_64 "./data/CA_icon_64.png"
#define ICON_128 "./data/CA_icon_128.png"

#define FILENAME "ca_image_"
#define FILETYPE "png"
#define SEEDFILE "seedfile.dat"

bool randomizeLength = FALSE, autoReset = FALSE;
bool savePics;

int x, y, timeouttag;
int fileSeed;
int numLines;
int WIDTH;
int HEIGHT;
int ruleChangeInterval;
int maxRuleChangeInterval;
int deadCount;

std::vector<int> rules;

GtkWidget *mainImage, *stopButton, *startButton, *ruleAdjust, *widthAdjust;
GtkWidget *rulesComboBox;
GtkWidget *autoResetButton, *randomizeLengthButton;

GdkPixbuf *imagebuf;
GdkPixbuf *buf, *buf2, *blackb, *black, *whiteb, *white, *color;

CellularAutomaton *ca1;



static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data);

void initGUI(int argc, char** argv);
void updateRule(GtkWidget *adjustor);
void redrawImage();
void resetImage(GtkWidget *data);

gboolean getLoopy(gpointer d);
gboolean stopLoopy(gpointer data);
gboolean getPic(gpointer data);

bool getRulesList(std::string dir, std::vector<std::string> & files, std::string suff);

void initCA();
void load_rules(std::string file);
void combo_selected(GtkWidget *widget, GtkWidget *ptr);
void clearRules(GtkWidget *widget);

gint toggleSave(gpointer data);
gint loopThrough(gpointer data);
gint autoResetFunction(gpointer data);
gint randomizeLengthFunction(gpointer data);


int main(int argc, char** argv) {
    //start gnome_init
    initGUI(argc, argv);

    //GUI components
    GtkWidget *window;
    GtkWidget *imageEventBox, *mainHBox, *sideBox, *quitButton,
              *settingFrame, *resetButton, *sideVBox, *widthFrame,
              *saveButton, *hSep, *label, *lowerSideVBox, *checkButton,
              *rulesFrame, *rulesClearButton, *rulesSetButton,
              *rulesHBox, *rulesVBox;
    GdkPixbuf *icon_buf_16, *icon_buf_32, *icon_buf_48,
              *icon_buf_64, *icon_buf_128, *bg;

    GList *icons = NULL;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    //load icons from files	
    icon_buf_16 = gdk_pixbuf_new_from_file(ICON_16, NULL);
    icon_buf_32 = gdk_pixbuf_new_from_file(ICON_32, NULL);
    icon_buf_48 = gdk_pixbuf_new_from_file(ICON_48, NULL);
    icon_buf_64 = gdk_pixbuf_new_from_file(ICON_64, NULL);
    icon_buf_128 = gdk_pixbuf_new_from_file(ICON_128, NULL);

    //tell gnome to use these icons
    icons = g_list_append(icons, icon_buf_16);
    icons = g_list_append(icons, icon_buf_32);
    icons = g_list_append(icons, icon_buf_48);
    icons = g_list_append(icons, icon_buf_64);
    icons = g_list_append(icons, icon_buf_128);
    gtk_window_set_icon_list(GTK_WINDOW(window), icons);

    //connect the delete_event signal to the delete_event function
    g_signal_connect (G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL);

    //here follows formatting, packing, etc of gtk widgets
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_title(GTK_WINDOW(window), "Cellular Automata");

    widthAdjust = gtk_spin_button_new_with_range(WIDTH_LOWER, WIDTH_UPPER, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widthAdjust), DEFAULT_WIDTH);
    widthFrame = gtk_frame_new("Set Width");
    gtk_container_add(GTK_CONTAINER(widthFrame), widthAdjust);
    gtk_container_set_border_width(GTK_CONTAINER(widthFrame), 10);

    ruleAdjust = gtk_spin_button_new_with_range(0, 255, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ruleAdjust), 30);
    settingFrame = gtk_frame_new("Set Rule #");
    gtk_container_add(GTK_CONTAINER(settingFrame), ruleAdjust);
    gtk_container_set_border_width (GTK_CONTAINER(settingFrame), 10);
    bg = gdk_pixbuf_new_from_file("./data/cellular_automata.png", NULL);
    imagebuf = gdk_pixbuf_scale_simple(bg, WIDTH, HEIGHT, GDK_INTERP_BILINEAR);
    mainImage = gtk_image_new_from_pixbuf(imagebuf);

    imageEventBox = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(imageEventBox), mainImage);
    mainHBox = gtk_hbox_new(FALSE, 0);
    sideVBox = gtk_vbox_new(FALSE, 0);
    lowerSideVBox = gtk_vbox_new(TRUE, 0);
    sideBox = gtk_vbox_new(TRUE, 0);
    checkButton = gtk_check_button_new_with_mnemonic("_Autosave Images");
    quitButton = gtk_button_new_from_stock (GTK_STOCK_QUIT);
    startButton = gtk_button_new_with_mnemonic("_Start");
    stopButton = gtk_button_new_with_mnemonic ("_Stop");
    saveButton = gtk_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_widget_set_sensitive(GTK_WIDGET(stopButton), FALSE);
    resetButton = gtk_button_new_with_mnemonic("_Reset");

    //connect the proper callback functions to the proper signals
    g_signal_connect_swapped (G_OBJECT(quitButton), "clicked", G_CALLBACK(gtk_main_quit), G_OBJECT (window));
    g_signal_connect_swapped (G_OBJECT(startButton),"clicked", G_CALLBACK(getLoopy), G_OBJECT(ruleAdjust));
    g_signal_connect_swapped (G_OBJECT(stopButton),"clicked", G_CALLBACK(stopLoopy), G_OBJECT(ruleAdjust));
    g_signal_connect_swapped (G_OBJECT(resetButton),"clicked", G_CALLBACK(resetImage), G_OBJECT(resetButton));
    g_signal_connect_swapped(G_OBJECT(ruleAdjust),"value-changed", G_CALLBACK(updateRule), GTK_WIDGET(ruleAdjust));
    g_signal_connect_swapped(G_OBJECT(widthAdjust),"value-changed", G_CALLBACK(initCA), GTK_WIDGET(widthAdjust));

    g_signal_connect_swapped(G_OBJECT(saveButton),"clicked", G_CALLBACK(getPic), GTK_WIDGET(saveButton));
    g_signal_connect_swapped(G_OBJECT(checkButton),"toggled", G_CALLBACK(toggleSave), GTK_WIDGET(checkButton));

    hSep = gtk_hseparator_new();
    label = gtk_label_new("Save Image");

    //load the meta-rule file names
    std::vector<std::string> rulesList;
    std::string rulesDir("./"), suff(".rules");
    getRulesList(rulesDir, rulesList, suff);

    rulesComboBox = gtk_combo_box_new_text();

    gtk_combo_box_append_text(GTK_COMBO_BOX(rulesComboBox), "Rule Set");

    for (uint i = 0; i < rulesList.size(); i++) {
        gtk_combo_box_append_text(GTK_COMBO_BOX(rulesComboBox), rulesList[i].c_str());
    }
    if (rulesList.size()) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(rulesComboBox), 0);
    }

    rulesFrame = gtk_frame_new("Rule Change Controls");
    rulesHBox = gtk_hbox_new(FALSE, 0);
    rulesVBox = gtk_vbox_new(TRUE, 0);	
    rulesClearButton = gtk_button_new_with_mnemonic("_Clear Rules Rule");
    rulesSetButton = gtk_button_new_with_mnemonic("Set _Rules Rule");

    g_signal_connect(G_OBJECT(rulesComboBox), "changed", G_CALLBACK(combo_selected), rulesFrame);
    g_signal_connect_swapped(G_OBJECT(rulesClearButton), "clicked", G_CALLBACK(clearRules), G_OBJECT(rulesClearButton));

    autoResetButton = gtk_check_button_new_with_mnemonic("_Auto Reset on Dead Lines");
    randomizeLengthButton = gtk_check_button_new_with_mnemonic("Randomize _Time Between Rule Changes");

    g_signal_connect_swapped(G_OBJECT(autoResetButton), "toggled", G_CALLBACK(autoResetFunction), G_OBJECT(autoResetButton));
    g_signal_connect_swapped(G_OBJECT(randomizeLengthButton), "toggled", G_CALLBACK(randomizeLengthFunction), G_OBJECT(randomizeLengthButton));

    //pack the widgets into their proper containers
    gtk_box_pack_start(GTK_BOX(rulesHBox), rulesClearButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rulesVBox), rulesComboBox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rulesVBox), rulesHBox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rulesVBox), autoResetButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rulesVBox), randomizeLengthButton, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(rulesFrame), rulesVBox);
    gtk_box_pack_start(GTK_BOX(sideVBox), settingFrame, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sideVBox), stopButton, FALSE, FALSE,0);
    gtk_box_pack_start(GTK_BOX(sideVBox), startButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sideVBox), resetButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sideBox), sideVBox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sideVBox), rulesFrame, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(lowerSideVBox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(lowerSideVBox), saveButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(lowerSideVBox), checkButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(lowerSideVBox), hSep, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(lowerSideVBox), quitButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sideBox), lowerSideVBox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mainHBox), sideBox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mainHBox), imageEventBox, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), mainHBox);

    gtk_widget_show_all(window);

    //load black and white pixels into buffers
    blackb = gdk_pixbuf_new_subpixbuf(imagebuf, 0, 0, 1, 1);
    black = gdk_pixbuf_copy(blackb);
    gdk_pixbuf_fill(black, 0x000000ff);
    whiteb = gdk_pixbuf_new_subpixbuf(imagebuf, 0, 0, 1, 1);
    white = gdk_pixbuf_copy(whiteb);
    gdk_pixbuf_fill(white, 0xffffffff);

    //setup the cellular automaton=========================================
    initCA();

    //run the main loop of the program
    gtk_main();
    exit(0);
}

//some GUI initialization 
//	gnome_program_init()
void initGUI(int argc, char** argv) {
    srand(time(NULL));
    WIDTH = DEFAULT_WIDTH;
    HEIGHT = DEFAULT_HEIGHT;
    std::ifstream seedFile;
    seedFile.open(SEEDFILE);
    seedFile >> fileSeed;
    seedFile.close();

    GnomeProgram *ca;
    ca = gnome_program_init("ca", "0.1", LIBGNOMEUI_MODULE, argc, argv, GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
}

//delete_event stops the program
static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data) {
    gtk_main_quit ();
    return FALSE;
}

//updateRule is called by a SPIN_BUTTON which changes the
// base 10 rule number
void updateRule(GtkWidget *adjustor) {
    ca1->setRule(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(adjustor)));
}

//redrawImage shifts the existing pixels down by one row, and draws
// the newest row at the top of the image
void redrawImage() {
    numLines++;
    //copy the existing image, without the topmost row
    buf = gdk_pixbuf_new_subpixbuf(imagebuf, 0, 0, WIDTH, HEIGHT - 1);
    //place that copy into a new buffer
    buf2 = gdk_pixbuf_copy(buf);
    //release the memory of the first buffer
    gdk_pixbuf_unref(buf);

    //copy the second buffer
    gdk_pixbuf_copy_area(buf2, 0, 0, WIDTH, HEIGHT - 1, imagebuf, 0, 1);
    //release the second buffer
    gdk_pixbuf_unref(buf2);

    //get the new row
    std::vector<bool> *data = ca1->getNewRow();

    //draw the new row on imagebuf
    for (unsigned int i = 0; i < ca1->getWidth(); i++) {
        if ((*data)[i]) {
            color = black;
        } else {
            color = white;
        }
        gdk_pixbuf_copy_area(color, 0, 0, 1, 1, imagebuf, i, 0);
    }
    //copy imagebuf onto the screen
    gtk_image_set_from_pixbuf(GTK_IMAGE(mainImage),imagebuf);
}

//this function is called after each timeout, which iterates the CA
gint loopThrough(gpointer data) {
    //if autoReset is toggled, find if row is all black or all white,
    // and reset if it is either
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoResetButton))) {
        int sum = ca1->getRowSum();
        if ((sum < ROW_LOWER_BOUND) || (sum > DEFAULT_WIDTH)) {
            ca1->resetCells();
        }
    }

    //if meta-rules have been loaded, apply them here
    if (rules.size() > 0) {
        if ((numLines % ruleChangeInterval == 0) && (numLines != 0)) {
            int seed = rand() % rules.size();
            ca1->setRule(rules[seed]);
            std::cout << "changing rule to " << rules[seed] << std::endl;
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(randomizeLengthButton))) {
                ruleChangeInterval=rand() % maxRuleChangeInterval + 1;
            }
        }
    }

    //calculate the new row, iterate the automaton
    ca1->calcNewRow();

    //redraw the image
    redrawImage();

    //if auto-saving pics, activate here
    if ((numLines % HEIGHT == 0) && savePics) {
        getPic(data);
    }
    return TRUE;
}

//resetImage fills imagebuf with white and copies it to the mainImage
void resetImage(GtkWidget *data) {
    gdk_pixbuf_fill(imagebuf, 0xffffffff);
    ca1->resetCells();
    gtk_image_set_from_pixbuf(GTK_IMAGE(mainImage), imagebuf);
}

//getLoopy sets up the timeout which iterates the cellular automaton
// this is called when startButton is clicked
gboolean getLoopy(gpointer d) {
    gboolean a = TRUE;

    //enable the stopButton
    gtk_widget_set_sensitive(GTK_WIDGET(stopButton), TRUE);
    gpointer data;
    int looptime = LOOPTIME;

    //store timeouttag so the timer can be stopped later
    timeouttag = g_timeout_add(looptime, loopThrough, data);

    //disable the start button
    gtk_widget_set_sensitive(GTK_WIDGET(startButton), FALSE);
    return a;
}

//stopLoopy stops the timer and swaps start and stop sensitivity
gboolean stopLoopy(gpointer data) {
    gtk_widget_set_sensitive(GTK_WIDGET(stopButton), FALSE);
    g_source_remove(timeouttag);
    gtk_widget_set_sensitive(GTK_WIDGET(startButton), TRUE);
    return FALSE;
}

//getPic saves the current imagebuf to a png file
gboolean getPic(gpointer data) {
    numLines = 1;
    std::string filename = FILENAME;
    std::ostringstream ss;
    ss << fileSeed;
    filename.append(ss.str());
    std::cout << "Saving image as " << filename << std::endl;
    filename.append(".");
    filename.append(FILETYPE);
    gdk_pixbuf_savev(imagebuf, filename.c_str(), FILETYPE, NULL, NULL, NULL);
    fileSeed++;

    //remove the old seedfile
    system("rm seedfile.dat");
    //rewrite the seedfile with an incremented count
    std::ofstream seedFile;
    seedFile.open(SEEDFILE);
    seedFile << fileSeed;
    seedFile.close();
    return FALSE;
}

//called in order to allow/disallow auto-saving images
gint toggleSave(gpointer data) {
    gint answer = 1;
    savePics = !savePics;
    return answer;
}

//sets up the cellularAutomata object
void initCA() {
    //initialization vector - allocate memory
    std::vector<bool> *init_vect = new std::vector<bool>;
    bool seed = FALSE;
    unsigned int width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widthAdjust));

    //populate the initialization vector entirely with off cells, with one
    // one cell at the midway through the vector
    for (unsigned int i = 0; i < width; i++) {
        init_vect->push_back(seed);
        if (i == (width / 2) || i == ((width / 2) + 1)) {
            seed = !seed;
        }
    }
    //create the CellularAutomata object
    ca1 = new CellularAutomaton;
    //pass the initialization vector
    ca1->seedCells(init_vect);
    delete init_vect;
    //set the cells
    ca1->resetCells();
    //initialize the rule
    ca1->setRule(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ruleAdjust)));
}

//getRulesList finds all *.rules files, and returns their names
bool getRulesList(std::string dir, std::vector<std::string> & files, std::string suff) {
    std::string buffer;
    uint suffsize = suff.size();
    DIR *directory;
    dirent *dirp;
    directory = opendir(dir.c_str());
    while ((dirp = readdir(directory)) != NULL) {
        buffer = std::string(dirp->d_name);
        if (buffer.size() > suffsize) {
            if (buffer.substr(buffer.size() - suffsize) == suff) {
                files.push_back(buffer);
            }
        }
    }
    closedir(directory);
    return FALSE;
}

//load_rules loads meta-rules from a file
void load_rules(std::string file) {
    if (file.substr(file.size() - 6) == ".rules") {
        rules.clear();
        std::ifstream ifs;
        ifs.open(file.c_str());
        int inputBuffer;
        ifs >> ruleChangeInterval;
        maxRuleChangeInterval = ruleChangeInterval;
        while (!ifs.eof()) {
            ifs >> inputBuffer;
            std::cout << inputBuffer << std::endl;
            rules.push_back(inputBuffer);
        }
        ifs.close();
    }
}

//callback for the meta-rules combo box
void combo_selected( GtkWidget *widget, GtkWidget *ptr) {
    std::string filename = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
    load_rules(filename);
}

//callback for clearing the meta-rules
void clearRules( GtkWidget *widget) {
    rules.clear();
}

//callback for toggling the autoReset feature
gint autoResetFunction(gpointer data) {
    gint answer = 1;
    autoReset = !autoReset;
    return answer;
}

//callback for toggling the randomizeLength feature
gint randomizeLengthFunction(gpointer data) {
    gint answer = 1;
    randomizeLength = !randomizeLength;
    return answer;
}
