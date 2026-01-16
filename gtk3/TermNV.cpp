/// -----------------------------------------------------------------
/// Jean-Pierre Laroche
/// projet 2018-08-08  (C) 2018   Copyright 2018  <laroche.jeanpierre@gmail.com>
/// but :
///			fast / flexible terminal / respecting the escape code
///			-no-pie EXÉCUTABLE  programme server terminal
///			More flexible than XTERM and more secure than public terminals
///			tool to develop a 5250/3270 or terminal semi-graphic application





#include <filesystem>
#include <sys/stat.h>
#include <vte-2.91/vte/vte.h>
#include <pango/pango.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <glib/gprintf.h>
///------------------------------------------
/// paramétrage spécifique
/// ex:
///------------------------------------------

const bool ALT_F4 = true;  // ALT_F4 ACTIVE


#define WORKPGM		"/usr/bin/nvim"




#define MESSAGE_ALT_F4 "Confirm destroy Application"


/// ----------------------------------------
/// par default
///-----------------------------------------
#define VTENAME "VTE-TERM3270"




//*******************************************************
// PROGRAME
//*******************************************************



GtkWidget	*window, *terminal;

GPid child_pid = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
//	function alphanumeric switch
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr unsigned long long int strswitch(const char* c_name, unsigned long long int l_hash = 0)
{
	/// string to int for switch
	return (*c_name == 0) ? l_hash : 101 * strswitch(c_name + 1) + *c_name;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Contrôle affectation programme
//////////////////////////////////////////////////////////////////////////////////////////////////
bool ctrlPgm(std::string v_TEXT)
{
	int b_pgm = false;


	std::filesystem::path p(v_TEXT.c_str());
			switch(strswitch(p.stem().c_str()))
				{
					case  strswitch("nvim")		: b_pgm =true;		break;
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



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///
///		traitement terminal GTK.
///
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void close_window()
{
	gtk_main_quit ();
}


///-------------------------------------
/// traitement ALT+F4
///-------------------------------------
gboolean key_press_ALTF4()
{

		GtkWidget *dialog;
		const gchar* _MSG_ =  MESSAGE_ALT_F4;

		dialog = gtk_message_dialog_new(
										GTK_WINDOW(window),
										GTK_DIALOG_MODAL,
										GTK_MESSAGE_QUESTION,
										GTK_BUTTONS_YES_NO,
										_MSG_,
										NULL,
										NULL);

		int result = gtk_dialog_run (GTK_DIALOG (dialog));

		gtk_widget_destroy(GTK_WIDGET(dialog));

		switch (result)
		{
			case  GTK_RESPONSE_YES:
									{
										close_window();
										return EXIT_FAILURE ;
										//break;
									}
			case GTK_RESPONSE_NO:
									// not active ALT_F4
									return GDK_EVENT_STOP;
									//break;
		}

	// not active ALT_F4
	return GDK_EVENT_STOP;
}

///-------------------------------------
/// traitement CTRL-Z
///-------------------------------------
gboolean key_press_CTRLZ(GtkWidget *widget, GdkEventKey *event)
{

	( void ) widget;
	if ( event->state & ( GDK_CONTROL_MASK ) )
	{
		if ( event->keyval == GDK_KEY_z ||  event->keyval == GDK_KEY_Z )
		{
			return GDK_EVENT_STOP;
		}

		if ( event->keyval == GDK_3BUTTON_PRESS )
		{
			return GDK_EVENT_STOP;
		}
	}
	return false ;
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
	gtk_widget_show_all(window);
}

/// -----------------------------------------------------------------------------
/// personalisation projet utilisant une terminal simplifier pour de la gestion
/// -----------------------------------------------------------------------------

void	init_Terminal()
{

	VteTerminal *VTE;

	#define VTEFONT	"Fira Code Light"


	char * font_terminal = (char*) malloc (50);

	/// confortable and extend numbers columns and rows
	// HELIX

	unsigned int COL=	126; // 120 cols  src   6 reserve helix
	unsigned int ROW =	42;  // 40  lines src   2 reserve helix


	//determines the maximum size for screens
	Display* d = XOpenDisplay(NULL);
	gint  s = DefaultScreen(d);
	// obsolète
	gint width  = DisplayWidth(d, s);;
	gint height = DisplayHeight(d, s);


	if ( width <= 1600 && height <=1024 ) {						// ex: 13"... 15" font 10
		g_sprintf(font_terminal,"%s  %s" , VTEFONT,"10");
		}
	else if ( width <= 1920 && height <=1080 ) {				// ex: 17"... 32" font 12
		g_sprintf(font_terminal,"%s  %s" , VTEFONT,"14");	   // xfce4 arandr 3  big screen 14
		}
	else if ( width > 1920 && width < 2560  ) {				  //  ex: 2560 x1600 > 27"  font 13
		g_sprintf(font_terminal,"%s  %s" , VTEFONT,"13");
	 }
	else if ( width >= 2560  ) {								//  ex: 3840 x2160 > 32"  font 13
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


/// -----------------------------------------------------------------------------
///  libvte function putting the terminal function active
/// -----------------------------------------------------------------------------


int main(int argc, char *argv[])
{
	std::setlocale(LC_ALL, "fr_FR.utf8");

	if (argc != 3) return EXIT_FAILURE;

	if ( FALSE == ctrlPgm(WORKPGM))		return EXIT_FAILURE;	// contrôle file exist helix

	/// -----------------------------------------------------------------------------
	/// -----------------------------------------------------------------------------
	/// 4 argument
	/// 0= le programe TermHX
	// 1= Project
	/// 2= directory working
	/// contrôle autorisation traitement --> protection
	/// BUTTON CLOSE off
	/// ALT-F4 CLOSE windows HX
	/// Button mini / maxi ON




	gchar *Title  = (char*) malloc (200);
	g_sprintf(Title,"Project: %s",(gchar*) argv[1]); // PROJECT

	gchar *wrkdir = (gchar*) argv[2];		 // lib work

/// ----------------------------------------------------
	// Tableau des variables d'environnement (doit se terminer par NULL)
	char *envp[] = {
		(gchar*)"TERM=xterm-256color",
		NULL  // Exemple d'autre variable
	};
	/// ----------------------------------------------------

	gchar *pgm_1[]  = {(gchar*)WORKPGM ,(gchar*)"-w", wrkdir,NULL};
	gchar ** command  = pgm_1;


/// -----------------------------------------------------------------------------
/// -----------------------------------------------------------------------------
/// -----------------------------------------------------------------------------


	// Initialise GTK, the window traditional work

	gtk_init(&argc,&argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	gtk_window_set_title(GTK_WINDOW(window),Title);

	gtk_window_set_resizable (GTK_WINDOW(window),true);			   // <--- spécifique helix

	if (ALT_F4 == true )
		gtk_window_set_deletable (GTK_WINDOW(window),true);
	else
		gtk_window_set_deletable (GTK_WINDOW(window),false);



	gtk_widget_set_events ( window, GDK_KEY_PRESS_MASK );

	/* Initialise the terminal */
	terminal = vte_terminal_new();

	// specific initialization of the terminal
	init_Terminal();


		// gpointer user_data


	gtk_window_set_title(GTK_WINDOW(window),Title);  // name PROJECT

	// Connect some signals
	g_signal_connect(GTK_WINDOW(window),"delete_event", G_CALLBACK(key_press_ALTF4), NULL);
	g_signal_connect(G_OBJECT(window),"key_press_event", G_CALLBACK(key_press_CTRLZ), NULL);

	g_signal_connect(terminal, "child-exited",  G_CALLBACK (close_window), NULL);
	g_signal_connect(terminal, "destroy",  G_CALLBACK (close_window), NULL);

	g_signal_connect(terminal, "resize-window", G_CALLBACK(on_resize_window),NULL);




	/* Put widgets together and run the main loop */
	gtk_container_add(GTK_CONTAINER(window), terminal);
	vte_terminal_spawn_async(
		VTE_TERMINAL(terminal), //VteTerminal *terminal
		VTE_PTY_DEFAULT, // VtePtyFlags pty_flags,

		wrkdir,			// const char *working_directory PROJECT ex; $home/myproject/src-zig
		command,		// command    call pgm and parm

		envp,			// environment

		(GSpawnFlags)(G_SPAWN_SEARCH_PATH |G_SPAWN_FILE_AND_ARGV_ZERO),				// spawn flags
		NULL,			// GSpawnChildSetupFunc child_setup,
		NULL,			// gpointer child_setup_data,
		NULL,			// GDestroyNotify child_setup_data_destroy,
		-1,				// int timeout
		NULL,			// GCancellable *cancellable,

		&term_spawn_callback,// VteTerminalSpawnAsyncCallback callback, get pid child

		NULL);



	gtk_widget_hide(window);			// hide = ignore flash
	gtk_widget_show_all(window);		// run



	gtk_main();

	return EXIT_SUCCESS;
}
