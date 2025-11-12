/// Jean-Pierre Laroche
/// projet 2018-08-08  (C) 2018   Copyright 2018  <laroche.jeanpierre@gmail.com>
/// but :     terminal rapide    / flexible / respectant le code escape
///            fast / flexible terminal / respecting the escape code
///            -no-pie EXÉCUTABLE  programme maitre mini server terminal
///            plus souple que XTERM et plus sécuritaire que les terminal public  pour des applicatifs
///            outil pour développer une application de type 5250 / 3270 ou terminal semi-graphic
///            tool to develop a 5250/3270 or terminal semi-graphic



#include <filesystem>
#include <sys/stat.h>
#include <X11/Xlib.h>

#include <vte/vte.h>
#include <adwaita.h>





///------------------------------------------
/// paramétrage spécifique
/// ex:
///------------------------------------------

#define VTENAME "VTE-TERM3270"

unsigned int COL=    168;    /// max 132

unsigned int ROW =    44;        /// max 42 including a line for the system


#define WORKPGM        "./defrep"


//*******************************************************
// PROGRAME
//*******************************************************



GtkWidget    *window, *terminal;
AdwDialog *dialog;
AdwAlertDialog *Alertdialog;
GPid child_pid = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    function alphanumeric switch
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr unsigned long long int strswitch(const char* c_name, unsigned long long int l_hash = 0)    /// string to int for switch
{

    return (*c_name == 0) ? l_hash : 101 * strswitch(c_name + 1) + *c_name;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    Contrôle affectation programme
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ctrlPgm(std::string v_TEXT)
{
    int b_pgm = FALSE;


    std::filesystem::path p(v_TEXT.c_str());
                                            switch(strswitch(p.stem().c_str()))
                                            {
                                                case  strswitch("defrep")        : b_pgm =TRUE;        break;
                                            }
    return b_pgm;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///        traitement terminal GTK.
///
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void close_window()
{
    gtk_window_destroy(GTK_WINDOW(window));
}


///-------------------------------------
/// traitement ALT+F4
///-------------------------------------
gboolean dialog_cb (AdwAlertDialog *dialog,  GAsyncResult   *result,  GtkWidget   *self)
{
  const char *response = adw_alert_dialog_choose_finish (dialog, result);

    if (g_str_equal(response, "cancel"))

    {
       // g_print("no\n");
       return TRUE;
    }

    if (g_str_equal(response, "scratch"))

    {
        gtk_window_destroy(GTK_WINDOW(self));
    }
    return FALSE;
}


static void showAlert_cb()
{

    dialog = adw_alert_dialog_new ("DANGER-CLOSE ?-!", NULL);
    adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel",  "_Cancel",
                                  "scratch", "_Scratch",
                                  NULL);
    Alertdialog = ADW_ALERT_DIALOG (dialog);
    adw_alert_dialog_choose (Alertdialog, GTK_WIDGET (window),
                           NULL, (GAsyncReadyCallback) dialog_cb, window);

    adw_dialog_present (dialog, window);

    g_signal_connect(window,"close-request", G_CALLBACK (showAlert_cb), window);

}
/// -----------------------------------------------------------------------------
/// personalisation projet utilisant une terminal simplifier pour de la gestion
/// -----------------------------------------------------------------------------

void	init_Terminal()
{

    //determines the maximum size for screens
    Display* d = XOpenDisplay(NULL);
    gint  s = DefaultScreen(d);
    // obsolète
    gint width  = DisplayWidth(d, s);;
    gint height = DisplayHeight(d, s);


    #define VTEFONT	"Source code Pro"
	VteTerminal *VTE;

	gchar * font_terminal = (char*) malloc (50);

	/// confortable and extend numbers columns and rows
	// HELIX


    /// confortable and extend numbers columns and rows

    if ( width <= 1600 && height >=1024 ) {                // ex: 13"... 15"
        sprintf(font_terminal,"%s  %s" , VTEFONT,"10");
        COL = 168;
        ROW = 44;
        }
    else if ( width <= 1920 && height >=1080 ) {           // ex: 17"... 32"
        sprintf(font_terminal,"%s  %s" , VTEFONT,"12");
        COL = 168;
        ROW = 44;
        }
    else if ( width > 1920  ) {                               //  ex: 2560 x1600 > 27"  font 13
        sprintf(font_terminal,"%s  %s" , VTEFONT,"13");          //  ex: 3840 x2160 > 32"  font 15
        COL = 168;
        ROW = 44;
    }
    //C

	// resize  title  font


	VTE = VTE_TERMINAL (terminal);

	vte_terminal_set_font (VTE,pango_font_description_from_string(font_terminal));	/// font use

	vte_terminal_set_size (VTE, COL, ROW);											/// size du terminal

    gtk_window_set_title(GTK_WINDOW(window), VTENAME);                              /// titre du terminal de base

	vte_terminal_set_scrollback_lines (VTE,0);		 								///	désactiver historique.

	vte_terminal_set_scroll_on_output(VTE,FALSE);									/// pas de défilement

	vte_terminal_set_scroll_on_keystroke(VTE,FALSE);								/// pas de défilement

	vte_terminal_set_mouse_autohide(VTE, TRUE);										/// hiden  mouse  keyboard Actif

	vte_terminal_set_cursor_blink_mode(VTE, VTE_CURSOR_BLINK_ON);					/// cursor blink on

	vte_terminal_set_cursor_shape(VTE,VTE_CURSOR_SHAPE_BLOCK);						/// define cursor 'block'


}


/// -----------------------------------------------------------------------------
/// Callback for vte_terminal_spawn_async    retrived PID terminal ONLY
/// -----------------------------------------------------------------------------
void term_spawn_callback(VteTerminal *terminal, GPid pid, GError *error, gpointer user_data)
{
        child_pid = pid;
}
/// -----------------------------------------------------------------------------
/// possibility to change the name of the terminal
/// -----------------------------------------------------------------------------


void on_title_changed(GtkWidget *terminal)
{
    const char *title;
    title = vte_terminal_get_termprop_string_by_id(VTE_TERMINAL(terminal), VTE_PROPERTY_ID_XTERM_TITLE, NULL);
    gtk_window_set_title (GTK_WINDOW(window), title);
}

/// -----------------------------------------------------------------------------
/// possibility to change the number of columns and rows
/// -----------------------------------------------------------------------------

void on_resize_window(GtkWidget *terminal, guint  _col, guint _row)
{

	vte_terminal_set_size (VTE_TERMINAL(terminal),_col,_row);

}





/// -----------------------------------------------------------------------------
///  libvte function putting the terminal function active
/// -----------------------------------------------------------------------------

inline bool exists_File (const std::string& name) {
    struct stat fileStat;
    if(stat(name.c_str(),&fileStat) < 0) return FALSE;      // is exist objet

    stat(name.c_str(),&fileStat);
    if (S_ISDIR(fileStat.st_mode) == 1 ) return FALSE;      // is dir

    if ((fileStat.st_mode & S_IXUSR) == 0 ) return FALSE;     // pas un executable

    return (stat (name.c_str(), &fileStat) == 0);
}


// programme linux pas d'extention windows ".exe"
inline bool extention_File(const std::string& name) {
        std::filesystem::path filePath = name.c_str();
        if (filePath.extension()!= "") return FALSE;
        return TRUE;
}

inline bool isDir_File(const std::string& name) {
        std::string strdir = std::filesystem::path(name.c_str()).parent_path();
        if (strdir.empty() ) return FALSE;
        return TRUE;
}





int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "");
    const gchar *dir;
    gchar ** command ;

/// -----------------------------------------------------------------------------
/// -----------------------------------------------------------------------------
/// -----------------------------------------------------------------------------
/// contrôle autorisation traitement --> protection



    gchar *arg_1[] = { (gchar*)WORKPGM,  NULL};
    gchar *arg_2[] = { (gchar*) argv[1], NULL};        /// arg[1] P_pgm
    // exemple P_Pgm (argv[1]) = ./programme

    if (argc == 1 )  {
        if ( FALSE == ctrlPgm(WORKPGM))                    return EXIT_FAILURE;    // contrôle file autorisation
        if ( FALSE == exists_File(WORKPGM) )             return EXIT_FAILURE;    // contrôle si programme
        dir = std::filesystem::path(WORKPGM).parent_path().c_str();
        command = arg_1;
    }
    if (argc == 2 )  {
        if ( FALSE == ctrlPgm((char*)argv[1]))            return EXIT_FAILURE;    // contrôle file autorisation
        if ( FALSE == extention_File((char*)argv[1]) )    return EXIT_FAILURE;    // contrôle extention
        if ( FALSE == isDir_File((char*)argv[1]) )         return EXIT_FAILURE;     // contrôle is directorie
        if ( FALSE == exists_File((char*)argv[1]) )        return EXIT_FAILURE;    // contrôle si programme
        dir = std::filesystem::path((const char*)(char*)argv[1]).parent_path().c_str();
        command = arg_2;
    }
    if (argc > 2)  return EXIT_FAILURE;




/// -----------------------------------------------------------------------------
/// -----------------------------------------------------------------------------
/// -----------------------------------------------------------------------------


    // Initialise GTK, the window traditional work

    adw_init ();
    window = gtk_window_new();

    //gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
    gtk_window_set_resizable (GTK_WINDOW(window),FALSE);


    gtk_window_set_deletable (GTK_WINDOW(window),TRUE);

    gtk_window_set_modal(GTK_WINDOW(window),TRUE);

    /* Initialise the terminal */
    terminal = vte_terminal_new();

    // specific initialization of the terminal
    init_Terminal();


    vte_terminal_spawn_async(
        VTE_TERMINAL(terminal), //VteTerminal *terminal
        VTE_PTY_DEFAULT, // VtePtyFlags pty_flags,

        dir,            // const char *working_directory
        command,        // command

        NULL,            // environment
        (GSpawnFlags)(G_SPAWN_SEARCH_PATH |G_SPAWN_FILE_AND_ARGV_ZERO),                // spawn flags
        NULL,            // GSpawnChildSetupFunc child_setup,
        NULL,            // gpointer child_setup_data,
        NULL,            // GDestroyNotify child_setup_data_destroy,
        -1,                // int timeout
        NULL,            // GCancellable *cancellable,

        &term_spawn_callback,// VteTerminalSpawnAsyncCallback callback, get pid child

        NULL);            // gpointer user_data
    gtk_window_set_child(GTK_WINDOW(window), terminal);

    // Connect some signals
    g_signal_connect(window,"close-request", G_CALLBACK (showAlert_cb), window);


    g_signal_connect(terminal, "child-exited",  G_CALLBACK (close_window), NULL);


    g_signal_connect(terminal, "window-title-changed", G_CALLBACK(on_title_changed), NULL);
    g_signal_connect(terminal, "resize-window", G_CALLBACK(on_resize_window),NULL);


    /* Put widgets together and run the main loop */

    gtk_window_present (GTK_WINDOW(window));

    while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
        g_main_context_iteration (NULL, TRUE);

    return EXIT_SUCCESS;
}
