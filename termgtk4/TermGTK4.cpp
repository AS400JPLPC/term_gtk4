#include <filesystem>
#include <sys/stat.h>
#include <X11/Xlib.h>


#include <vte/vte.h>
#include <glib/gprintf.h>
#include <gtk-4.0/gdk/gdk.h>

GtkWidget *window, *terminal;
GtkAlertDialog *Alertdialog;

GPid child_pid = 0;


const bool ALT_F4 = true;


#define WORKPGM		"hx"
#define WORKENV		"~/.helix"





/////////////////////////////////////////////////////////////////////////////////////////////////////
//	function alphanumeric switch
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr unsigned long long int strswitch(const char* c_name, unsigned long long int l_hash = 0)
{
	/// string to int for switch
	return (*c_name == 0) ? l_hash : 101 * strswitch(c_name + 1) + *c_name;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Contrôle affectation programme
///////////////////////////////////////////////////////////////////////////////////////////////////
bool ctrlPgm(std::string v_TEXT)
{
	int b_pgm = FALSE;


	std::filesystem::path p(v_TEXT.c_str());
				switch(strswitch(p.stem().c_str()))
					{
						case  strswitch("hx")		: b_pgm =TRUE;		break;
				}
	return b_pgm;
}

/// -----------------------------------------------------------------------------
///  libvte function putting the terminal function active
/// -----------------------------------------------------------------------------

inline bool exists_File (const std::string& name) {
	struct stat fileStat;
	if(stat(name.c_str(),&fileStat) != 0 ) { printf("\nNot exist -> %s\n",name.c_str()); return false;} 	// is exist objet
	return true;
}


// programme linux pas d'extention windows ".exe"
inline bool extention_File(const std::string& name) {
		std::filesystem::path filePath = name.c_str();
		if (filePath.extension()!= "") { printf("\nExtension->invalide  %s\n",name.c_str()); return false; }
		return true;
}

inline bool isDir_File(const std::string& name) {
		std::string strdir = std::filesystem::path(name.c_str()).parent_path();
		if (strdir.empty() ) { printf("\nDIR ->invalide  %s\n",strdir.c_str()); return false;}
		return true;
}



/// -----------------------------------------------------------------------------
/// Callback for vte_terminal_spawn_async	retrived PID terminal ONLY
/// -----------------------------------------------------------------------------
void term_spawn_callback(VteTerminal *terminal, GPid pid, GError *error, gpointer user_data)
{
	child_pid = pid;
}

/// -----------------------------------------------------------------------------
/// possibility to change the number of columns and rows
/// -----------------------------------------------------------------------------

void on_resize_window(GtkWidget *terminal, guint  _col, guint _row)
{
	vte_terminal_set_size (VTE_TERMINAL(terminal),_col,_row);
	gtk_window_set_default_size(GTK_WINDOW(window),-1,-1);
    gtk_window_present(GTK_WINDOW(window));
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


///-------------------------------------
/// traitement ALT+F4
///-------------------------------------

void close_window() {
	gtk_window_destroy(GTK_WINDOW(window));
}


gboolean dialog_cb (GObject *source_object, GAsyncResult *res, gpointer user_data) {


        GtkAlertDialog *dialog = GTK_ALERT_DIALOG (source_object);
	    GError *err = NULL;
	    int button= gtk_alert_dialog_choose_finish (GTK_ALERT_DIALOG(dialog), res, &err);


	    if (button == 0)

	    {
		    gtk_window_destroy(GTK_WINDOW(window));
	    }


	    if (button == 1)

	    {
		    gtk_window_present (GTK_WINDOW(window));

		    return TRUE;
	    }

	return FALSE;
}


static void showAlert_cb()
{

    gtk_alert_dialog_choose (GTK_ALERT_DIALOG(Alertdialog), GTK_WINDOW(window),
						NULL, (GAsyncReadyCallback) dialog_cb, NULL);

	gtk_window_present (GTK_WINDOW(window));
	g_signal_connect(window,"close-request", G_CALLBACK (showAlert_cb), NULL);

}
/// -----------------------------------------------------------------------------
/// personalisation projet utilisant une terminal simplifier pour de la gestion
/// -----------------------------------------------------------------------------

void	init_Terminal()
{


	VteTerminal *VTE;

    #define VTEFONT	"SourceCodePro"


	gchar * font_terminal = (char*) malloc (50);

	/// confortable and extend numbers columns and rows
	// HELIX

    unsigned int COL=	126; // 120 cols  src
    unsigned int ROW =	40;  // 38  lines src


    //determines the maximum size for screens
    Display* d = XOpenDisplay(NULL);
    gint  s = DefaultScreen(d);
    // obsolète
    gint width  = DisplayWidth(d, s);;
    gint height = DisplayHeight(d, s);


    if ( width <= 1600 && height >=1024 ) {                // ex: 13"... 15"
        g_sprintf(font_terminal,"%s  %s" , VTEFONT,"10");
        }
    else if ( width <= 1920 && height >=1080 ) {           // ex: 17"... 32"
        g_sprintf(font_terminal,"%s  %s" , VTEFONT,"12");
        }
    else if ( width > 1920 && width<= 2560  ) {            //  ex: 2560 x1600 > 27"  font 13
        g_sprintf(font_terminal,"%s  %s" , VTEFONT,"13");
    }
    else if ( width > 2560  ) {                            //  ex: 3840 x2160 > 32"  font 14
        g_sprintf(font_terminal,"%s  %s" , VTEFONT,"13");
    }




	// resize  title  font
	VTE = VTE_TERMINAL (terminal);

	vte_terminal_set_font (VTE,pango_font_description_from_string(font_terminal));	/// font use

	vte_terminal_set_size (VTE, COL, ROW);											/// size du terminal

	vte_terminal_set_scrollback_lines (VTE,0);		 								///	désactiver historique.

	vte_terminal_set_scroll_on_output(VTE,FALSE);									/// pas de défilement

	vte_terminal_set_scroll_on_keystroke(VTE,FALSE);								/// pas de défilement

	vte_terminal_set_mouse_autohide(VTE, TRUE);										/// hiden  mouse  keyboard Actif

	vte_terminal_set_cursor_blink_mode(VTE, VTE_CURSOR_BLINK_ON);					/// cursor blink on

	vte_terminal_set_cursor_shape(VTE,VTE_CURSOR_SHAPE_BLOCK);						/// define cursor 'block'

}

int main (int   argc,   char *argv[])  {


	std::setlocale(LC_ALL, "fr_FR.utf8");

    if (argc != 3) return EXIT_FAILURE;

    if ( FALSE == ctrlPgm(WORKPGM))		return EXIT_FAILURE;	// contrôle file exist helix


    /// -----------------------------------------------------------------------------
    /// -----------------------------------------------------------------------------
    /// 4 argument
    /// 0= le programe TermHX
    /// 1= Project
    /// 2= directory working
    /// contrôle autorisation traitement --> protection
    /// BUTTON CLOSE off
    /// ALT-F4 CLOSE windows HX
    /// Button mini / maxi ON





	gchar *Title  = (char*) malloc (200);
	g_sprintf(Title,"Project: %s",(gchar*) argv[1]); // PROJECT

	gchar *wrkdir = (gchar*) argv[2];         // lib work

/// ----------------------------------------------------
    // Définir le PATH (ajoute ton chemin au PATH existant)
    const char *env_path = g_getenv("PATH");
    char *new_path = g_strdup_printf("PATH=%s:%s", env_path, WORKENV);

    // Tableau des variables d'environnement (doit se terminer par NULL)
    char *envp[] = {
        new_path,  // Ajoute ton chemin au PATH
        (gchar*)"TERM=xterm-256color"  // Exemple d'autre variable
    };
    /// ----------------------------------------------------


    gchar *pgm_1[]  = {(gchar*)WORKPGM ,(gchar*)"-w", wrkdir,NULL};    // hx
    gchar ** command  = pgm_1;

//==============================================================================================
//==============================================================================================
	gtk_init ();
	window = gtk_window_new ();



	gtk_window_set_title(GTK_WINDOW(window),Title);

	gtk_window_set_resizable (GTK_WINDOW(window),TRUE);

	gtk_window_set_modal(GTK_WINDOW(window),TRUE);

    if (ALT_F4 == true ) gtk_window_set_deletable (GTK_WINDOW(window),true);
    else gtk_window_set_deletable (GTK_WINDOW(window),false);



	// specific initialization of the terminal
	terminal = vte_terminal_new();
	init_Terminal();


	vte_terminal_spawn_async(
		VTE_TERMINAL(terminal), //VteTerminal *terminal
		VTE_PTY_DEFAULT, // VtePtyFlags pty_flags,

		wrkdir ,		// const char *working_directory PROJECT
		command ,		// command    call pgm and parm
		envp ,			// environment
		(GSpawnFlags)(G_SPAWN_SEARCH_PATH |G_SPAWN_FILE_AND_ARGV_ZERO),		// spawn flags
		NULL,			// GSpawnChildSetupFunc child_setup,
		NULL,			// gpointer child_setup_data,
		NULL,			// GDestroyNotify child_setup_data_destroy,
		-1,				// int timeout
		NULL,			// GCancellable *cancellable,

		&term_spawn_callback,   // VteTerminalSpawnAsyncCallback callback, get pid child

		NULL
    );

	gtk_window_set_child(GTK_WINDOW(window), terminal);

	Alertdialog = gtk_alert_dialog_new("confirm destroy Application");
	const char* buttons[] = {"YES","NO",NULL};
	gtk_alert_dialog_set_detail (GTK_ALERT_DIALOG(Alertdialog), "Please be careful");
	gtk_alert_dialog_set_buttons (GTK_ALERT_DIALOG(Alertdialog), buttons);
	gtk_alert_dialog_set_default_button ( GTK_ALERT_DIALOG(Alertdialog), 1);
    gtk_alert_dialog_set_modal(GTK_ALERT_DIALOG(Alertdialog),TRUE);


    g_signal_connect(window,"close-request", G_CALLBACK (showAlert_cb), NULL);
	g_signal_connect(terminal,"child-exited", G_CALLBACK (close_window), NULL);
	g_signal_connect(terminal, "resize-window", G_CALLBACK(on_resize_window),NULL);
	g_signal_connect(terminal, "window-title-changed", G_CALLBACK(on_title_changed), NULL);





	gtk_window_present (GTK_WINDOW(window));

	while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
		g_main_context_iteration (NULL, TRUE);



	return EXIT_SUCCESS;
}
