#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include "close.h"
#include "testwin.h"
#include "interface.h"
#include "client_params.h"

int wait_key = 0;
int iLocation [2];
char cBoardLoc[10][10] = { {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'}, {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'},
    {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'}, {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'},
    {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'}, {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'},
    {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'}, {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'},
    {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'}, {'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E'}
};
char cTurn = 'X';

enum
{
    LIST_ITEM = 0,
    N_COLUMNS
};

void Show_message(GtkWidget *parent, GtkMessageType type, char *mms, char *content)
{
    GtkWidget *mdialog;
    mdialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                     GTK_DIALOG_MODAL,
                                     type,
                                     GTK_BUTTONS_OK,
                                     "%s", mms);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s",  content);
    gtk_dialog_run(GTK_DIALOG(mdialog));
    gtk_widget_destroy(mdialog);
}

static gboolean get_x_loc (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    iLocation [0] = (int) data;

    return FALSE;
}

static gboolean get_y_loc (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    iLocation [1] = (int) data;

    return FALSE;
}

static gboolean make_move (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    int x = 0, y = 0;
    int iXPos = 0;
    int iYPos = 0;
    GtkWidget *player_img;
    GtkWidget *dialog_turn, *dialog_invalid;
    GtkResponseType result;
    char cImgLoc [16] = "./images/";
    strcat (cImgLoc, &cTurn);
    strcat (cImgLoc, ".png");
    if(flag_turn == 1)
    {
        if (cBoardLoc [iLocation [0]][iLocation [1]] == 'E')
        {

            cBoardLoc [iLocation [0]][iLocation [1]] = cTurn;
            send_play();
            // thực hiện gửi nhận server chỗ này
            while (x < iLocation [0])
            {
                iXPos += 60;
                x++;
            }

            while (y < iLocation [1])
            {
                iYPos += 60;
                y++;
            }

            player_img = gtk_image_new_from_file (cImgLoc);
            gtk_fixed_put (GTK_FIXED (fixed_main), player_img, iXPos, iYPos);
            gtk_widget_show (player_img);

        }
        else
        {
            g_print ("Invalid move!\n");
            Show_message(window_main, GTK_MESSAGE_ERROR, "ERROR", "An invalid move, try again!");
        }
    }
    else
    {
        Show_message(window_main, GTK_MESSAGE_ERROR, "ERROR", "Not your turn, wait opponent");
    }
    return TRUE;
}

void set_move(char *data)
{
    int x = 0, y = 0, x2, y2;
    int iXPos = 0;
    int iYPos = 0;
    GtkWidget *player_img;
    GtkWidget *dialog_win, *dialog_invalid;
    GtkResponseType result;
    char cImgLoc [16] = "./images/O.png";

    int temp_data = atoi(data);
    x2 = temp_data % 10;
    y2 = (temp_data - x2) / 10;

    if (cBoardLoc [x2][y2] == 'E')
    {
        cBoardLoc [x2][y2] = 'O';
        // thực hiện gửi nhận server chỗ này
        while (x < x2)
        {
            iXPos += 60;
            x++;
        }

        while (y < y2)
        {
            iYPos += 60;
            y++;
        }

        player_img = gtk_image_new_from_file (cImgLoc);
        gtk_fixed_put (GTK_FIXED (fixed_main), player_img, iXPos, iYPos);
        gtk_widget_show (player_img);

    }
}

void convert_room_detail(char *data)
{
    int i = 0, j, k = 0;
    char element[10];
    while(data[i])
    {
        memset(element, 0, strlen(element));
        j = 0;
        while(data[i] != '-')
        {
            element[j++] = data[i++];
        }
        room_arr[k].id = atoi(element);
        memset(element, 0, strlen(element));
        j = 0;
        while(data[i] != '#')
        {
            element[j++] = data[i++];
        }
        room_arr[k].client_num = atoi(element);
        k++; // room index
        i++; // string index
    }
}

void set_message (char *entry_text)
{
    gchar *view_text;
    char text[1000];

    GtkTextBuffer *buffer;
    GtkTextIter start, end, iter;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(view)));
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

    gtk_text_buffer_insert(buffer, &iter, "", -1);
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    view_text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    if(strcmp(view_text, "") != 0)
    {
        strcpy(text, view_text);
        strcat(text, "\n");
        strcat(text, entry_text);
    }
    else
    {
        strcpy(text, entry_text);
    }

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    if (buffer == NULL)
        buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(buffer, text, -1);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), buffer);
    gtk_entry_set_text(GTK_ENTRY(entry_mes), "");
}

void set_world_message (char *entry_text)
{
    gchar *view_text;
    char text[1000];

    GtkTextBuffer *buffer;
    GtkTextIter start, end, iter;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(world_message_view)));
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

    gtk_text_buffer_insert(buffer, &iter, "", -1);
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    view_text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    if(strcmp(view_text, "") != 0)
    {
        strcpy(text, view_text);
        strcat(text, "\n");
        strcat(text, entry_text);
    }
    else
    {
        strcpy(text, entry_text);
    }

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(world_message_view));
    if (buffer == NULL)
        buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(buffer, text, -1);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(world_message_view), buffer);
    gtk_entry_set_text(GTK_ENTRY(world_entry_message), "");

}

void hide_room_select()
{
    gtk_widget_hide(window_choose_room);
    int x, y;
    for (y = 0; y <= 9; y++)
    {
        for (x = 0; x <= 9; x++)
        {
            cBoardLoc [x][y] = 'E';
        }
    }
}

void wait_player_window(char *data)
{
    if(strcmp(data, "wait_key") == 0)
    {
        wait_key = 1;

        window_wait = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        g_signal_connect (G_OBJECT (window_wait), "delete_event", G_CALLBACK (delete_event), NULL);
        g_signal_connect (G_OBJECT (window_wait), "destroy", G_CALLBACK (destroy), NULL);
        gtk_window_set_title (GTK_WINDOW (window_wait), "Caro");
        gtk_window_set_default_size(GTK_WINDOW(window_wait), 600, 300);
        gtk_window_set_position(GTK_WINDOW(window_wait), GTK_WIN_POS_CENTER);
        gtk_container_set_border_width (GTK_CONTAINER (window_wait), 15);

        fixed_wait = gtk_fixed_new ();
        gtk_container_add (GTK_CONTAINER (window_wait), fixed_wait);
        label_wait = gtk_label_new("");
        const char *format = "<span font=\"25\" color=\"blue\" >\%s</span>";
        char *markup;

        markup = g_markup_printf_escaped (format, "Waiting anothor player");
        gtk_label_set_markup (GTK_LABEL (label_wait), markup);
        g_free (markup);
        gtk_fixed_put (GTK_FIXED (fixed_wait), label_wait, 140, 100);

        GtkWidget *cancel_button = gtk_button_new_with_label ("Cancel");
        gtk_widget_set_size_request(cancel_button, 100, 50);
        gtk_fixed_put (GTK_FIXED (fixed_wait), cancel_button, 250, 200);

        g_signal_connect(G_OBJECT(cancel_button), "clicked", G_CALLBACK(on_cancel_button_clicked), NULL);

        gtk_widget_show(label_wait);
        gtk_widget_show(cancel_button);
        gtk_widget_show (fixed_wait);
        gtk_widget_show (window_wait);
    }
}

void init_play_window(char *data)
{
    int x, y;
    int iXPos = 0;
    int iYPos = 0;

    GtkWidget *image_board;
    GtkWidget *scrolling, *label_player;
    GtkWidget *send_button, *newgame_button, *back_button;

    if(wait_key == 1)
    {
        gtk_widget_hide(window_wait);
    }

    window_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (window_main), "delete_event", G_CALLBACK (delete_event), NULL);
    g_signal_connect (G_OBJECT (window_main), "destroy", G_CALLBACK (destroy), NULL);
    gtk_window_set_title (GTK_WINDOW (window_main), "Caro");
    gtk_window_set_position(GTK_WINDOW(window_main), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width (GTK_CONTAINER (window_main), 15);


    image_board = gtk_image_new_from_file ("./images/caro.png");

    fixed_main = gtk_fixed_new ();
    gtk_container_add (GTK_CONTAINER (window_main), fixed_main);
    gtk_fixed_put (GTK_FIXED (fixed_main), image_board, 0, 0);

    view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
    gtk_text_view_set_editable (GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW(view), 3);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW(view), 3);

    scrolling = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolling), view);
    gtk_widget_set_size_request(scrolling, 300, 350);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolling), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_fixed_put (GTK_FIXED (fixed_main), scrolling, 620, 10);


    entry_mes = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (entry_mes), 150);
    gtk_entry_set_placeholder_text (GTK_ENTRY (entry_mes), "Type your message ...");
    gtk_widget_set_size_request(entry_mes, 250, 35);
    gtk_fixed_put (GTK_FIXED (fixed_main), entry_mes, 620, 375);

    send_button = gtk_button_new_with_label ("Send");
    gtk_widget_set_size_request(send_button, 40, 35);
    gtk_fixed_put (GTK_FIXED (fixed_main), send_button, 880, 375);

    back_button = gtk_button_new_with_label ("Back");
    gtk_widget_set_size_request(back_button, 100, 50);
    gtk_fixed_put (GTK_FIXED (fixed_main), back_button, 700, 500);

    if(strcmp(data, "wait_key") != 0)
    {

        label_player = gtk_label_new("");
        const char *format = "<span font=\"15\" color=\"red\">Your opponent: \%s</span>";
        char *markup;

        markup = g_markup_printf_escaped (format, data);
        gtk_label_set_markup (GTK_LABEL (label_player), markup);
        g_free (markup);
        gtk_fixed_put (GTK_FIXED (fixed_main), label_player, 630, 440);
        gtk_widget_show (label_player);

    }

    g_signal_connect(G_OBJECT(back_button), "clicked", G_CALLBACK(on_back_button_clicked), NULL);

    g_signal_connect(G_OBJECT(entry_mes), "activate", G_CALLBACK(on_button_send_clicked), NULL);

    g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(on_button_send_clicked), NULL);


    for (y = 0; y <= 9; y++)
    {
        for (x = 0; x <= 9; x++)
        {
            eventbox_main[x][y] = gtk_event_box_new ();
            g_signal_connect (G_OBJECT (eventbox_main[x][y]), "button_press_event", G_CALLBACK (get_x_loc), (gpointer) x);
            g_signal_connect (G_OBJECT (eventbox_main[x][y]), "button_press_event", G_CALLBACK (get_y_loc), (gpointer) y);
            g_signal_connect (G_OBJECT (eventbox_main[x][y]), "button_press_event", G_CALLBACK (make_move), NULL);
            gtk_widget_set_size_request (eventbox_main[x][y], 60, 60);
            gtk_fixed_put (GTK_FIXED (fixed_main), eventbox_main[x][y], iXPos, iYPos);
            gtk_event_box_set_visible_window (GTK_EVENT_BOX (eventbox_main[x][y]), FALSE);
            gtk_widget_show (eventbox_main[x][y]);

            iXPos += 60;
        }
        iYPos += 60;
        iXPos = 0;
    }

    gtk_widget_show (image_board);
    gtk_widget_show (fixed_main);
    gtk_widget_show (view);
    gtk_widget_show (scrolling);
    gtk_widget_show (send_button);
    gtk_widget_show (back_button);
    gtk_widget_show (entry_mes);
    gtk_widget_show (window_main);

}

void init_home_window ()
{
    GtkWidget *image;
    GtkWidget *exit_button;
    GtkWidget *scrolling;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkTreeSelection *selection;

    window_home = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (window_home), "delete_event", G_CALLBACK (delete_event), NULL);
    g_signal_connect (G_OBJECT (window_home), "destroy", G_CALLBACK (destroy), NULL);
    gtk_window_set_title (GTK_WINDOW (window_home), "Caro");
    gtk_window_set_default_size(GTK_WINDOW(window_home), 1000, 600);
    gtk_window_set_position(GTK_WINDOW(window_home), GTK_WIN_POS_CENTER);

    fixed_home = gtk_fixed_new ();
    gtk_container_add (GTK_CONTAINER (window_home), fixed_home);

    image = gtk_image_new_from_file("./images/background.png");
    gtk_container_add(GTK_CONTAINER(fixed_home), image);

    entry_name = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (entry_name), 150);
    gtk_entry_set_placeholder_text (GTK_ENTRY (entry_name), "Type your nickname ...");
    gtk_widget_set_size_request(entry_name, 250, 40);
    gtk_fixed_put (GTK_FIXED (fixed_home), entry_name, 190, 215);

    set_button = gtk_button_new_with_label ("Set");
    gtk_widget_set_size_request(set_button, 60, 41);
    gtk_fixed_put (GTK_FIXED (fixed_home), set_button, 480, 215);

    list = gtk_tree_view_new();
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);

    vbox = gtk_vbox_new(FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), list, TRUE, TRUE, 5);

    label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);

    scrolling = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolling), vbox);
    gtk_widget_set_size_request(scrolling, 180, 220);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolling), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_fixed_put (GTK_FIXED (fixed_home), scrolling, 760, 80);

    // world message
    world_message_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(world_message_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable (GTK_TEXT_VIEW(world_message_view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(world_message_view), FALSE);
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW(world_message_view), 3);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW(world_message_view), 3);


    world_msg_scrolling = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(world_msg_scrolling), world_message_view);
    gtk_widget_set_size_request(world_msg_scrolling, 300, 350);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolling), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_fixed_put (GTK_FIXED (fixed_home), world_msg_scrolling, 650, 80);


    world_entry_message = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (world_entry_message), 150);
    gtk_entry_set_placeholder_text (GTK_ENTRY (world_entry_message), "Type your message ...");
    gtk_widget_set_size_request(world_entry_message, 250, 35);
    gtk_fixed_put (GTK_FIXED (fixed_home), world_entry_message, 650, 450);

    send_world_message_button = gtk_button_new_with_label ("Send");
    gtk_widget_set_size_request(send_world_message_button, 40, 35);
    gtk_fixed_put (GTK_FIXED (fixed_home), send_world_message_button, 910, 450);

    // world msg

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));

    exit_button = gtk_button_new_with_label ("Exit");
    gtk_widget_set_size_request(exit_button, 80, 40);
    gtk_fixed_put (GTK_FIXED (fixed_home), exit_button, 30, 530);

    choose_room_button = gtk_button_new_with_label ("Choose room");
    gtk_widget_set_size_request(choose_room_button, 100, 41);
    gtk_fixed_put (GTK_FIXED (fixed_home), choose_room_button, 480, 210);


    g_signal_connect (G_OBJECT(choose_room_button), "clicked", G_CALLBACK(on_choose_room_button_clicked), NULL);

    g_signal_connect (G_OBJECT(exit_button), "clicked", G_CALLBACK(on_exit_button_clicked), NULL);

    g_signal_connect(G_OBJECT(set_button), "clicked", G_CALLBACK(on_set_button_clicked), NULL);

    g_signal_connect(G_OBJECT(send_world_message_button), "clicked", G_CALLBACK(on_world_button_send_clicked), NULL);


    gtk_widget_show (fixed_home);
    gtk_widget_show (image);
    gtk_widget_show (entry_name);
    gtk_widget_show (set_button);
    gtk_widget_show (vbox);
    gtk_widget_show (label);

    gtk_widget_show (exit_button);
    gtk_widget_show (window_home);

}

void init_choose_room_window(char *data)
{
    window_choose_room = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (window_choose_room), "delete_event", G_CALLBACK (delete_event), NULL);
    g_signal_connect (G_OBJECT (window_choose_room), "destroy", G_CALLBACK (destroy), NULL);
    gtk_window_set_title (GTK_WINDOW (window_choose_room), "Caro");
    gtk_window_set_default_size(GTK_WINDOW(window_choose_room), 1000, 600);
    gtk_window_set_position(GTK_WINDOW(window_choose_room), GTK_WIN_POS_CENTER);

    table = gtk_table_new(7, 4, TRUE);
    gtk_container_add(GTK_CONTAINER(window_choose_room), table);

    GtkWidget *label_room;
    label_room = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_room), "<b>Choose a room</b>");
    gtk_table_attach_defaults(GTK_TABLE(table), label_room, 1, 3, 0, 1);
    gtk_widget_show(label_room);
    convert_room_detail(data);
    char row[256];
    for(int i = 0; i < ROOM_NUM; i++)
    {
        memset(row, 0, strlen(row));
        sprintf(row, "Room %d - %d/2 player\n", room_arr[i].id, room_arr[i].client_num);

        button_room[i] = gtk_button_new_with_label(row);

        gtk_table_attach_defaults(GTK_TABLE(table), button_room[i], 1, 3, i + 1, i + 2);

        g_signal_connect(G_OBJECT(button_room[i]), "clicked", G_CALLBACK(on_room_button_clicked), (gpointer) i);

        gtk_widget_show(button_room[i]);
    }
    gtk_widget_show (table);
    gtk_widget_show (window_choose_room);
}

void add_to_list(GtkWidget *tlist, const gchar *str)
{

    GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model
                           (GTK_TREE_VIEW(tlist)));

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, LIST_ITEM, str, -1);
}

void append_item(GtkWidget *widget, gpointer entry)
{

    GtkListStore *store;
    GtkTreeIter iter;

    const gchar *str = gtk_entry_get_text(entry);

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, LIST_ITEM, str, -1);

}

void remove_item(GtkWidget *widget, gpointer selection)
{

    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter  iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));

    if (gtk_tree_model_get_iter_first(model, &iter) == FALSE)
    {
        return;
    }

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),
                                        &model, &iter))
    {
        gtk_list_store_remove(store, &iter);
    }
}

void remove_all(GtkWidget *widget, gpointer selection)
{

    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter  iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));

    if (gtk_tree_model_get_iter_first(model, &iter) == FALSE)
    {
        return;
    }

    gtk_list_store_clear(store);
}


void init_list(GtkWidget *tlist)
{

    GtkCellRenderer    *renderer;
    GtkTreeViewColumn  *column;
    GtkListStore       *store;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("List Item", renderer, "text", LIST_ITEM, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tlist), column);

    store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);

    gtk_tree_view_set_model(GTK_TREE_VIEW(tlist), GTK_TREE_MODEL(store));

    g_object_unref(store);
}

void on_button_send_clicked(GtkWidget *w, gpointer data)
{
    gchar *entry_text, *view_text;
    char text[10000];
    entry_text = gtk_entry_get_text(GTK_ENTRY(entry_mes));
    if(strcmp(entry_text, "") != 0)
    {
        send_msg(entry_text);
        GtkTextBuffer *buffer;
        GtkTextIter start, end, iter;

        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(view)));
        gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

        gtk_text_buffer_insert(buffer, &iter, "", -1);
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        view_text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

        if(strcmp(view_text, "") != 0)
        {
            strcpy(text, view_text);
            strcat(text, "\n");
            strcat(text, "You: ");
            strcat(text, entry_text);
        }
        else
        {
            strcpy(text, "You: ");
            strcat(text, entry_text);
        }

        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
        if (buffer == NULL)
            buffer = gtk_text_buffer_new(NULL);
        gtk_text_buffer_set_text(buffer, text, -1);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), buffer);
        gtk_entry_set_text(GTK_ENTRY(entry_mes), "");
    }
}

void on_world_button_send_clicked(GtkWidget *w, gpointer data)
{
    gchar *entry_text, *view_text;
    char text[10000];
    entry_text = gtk_entry_get_text(GTK_ENTRY(world_entry_message));
    if(strcmp(entry_text, "") != 0)
    {
        send_world_message(entry_text);
        GtkTextBuffer *buffer;
        GtkTextIter start, end, iter;

        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(world_message_view)));
        gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

        gtk_text_buffer_insert(buffer, &iter, "", -1);
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        view_text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

        if(strcmp(view_text, "") != 0)
        {
            strcpy(text, view_text);
            strcat(text, "\n");
            strcat(text, "You: ");
            strcat(text, entry_text);
        }
        else
        {
            strcpy(text, "You: ");
            strcat(text, entry_text);
        }

        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(world_message_view));
        if (buffer == NULL)
            buffer = gtk_text_buffer_new(NULL);
        gtk_text_buffer_set_text(buffer, text, -1);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(world_message_view), buffer);
        gtk_entry_set_text(GTK_ENTRY(world_entry_message), "");
    }
}

void on_choose_room_button_clicked()
{
    send_choose_room();
}

void on_room_button_clicked(GtkWidget *widget, gpointer data)
{
    int room_index = (int) data;
    printf("%d\n", room_index);
    char buff_temp[256];
    sprintf(buff_temp, "%d", room_index);
    send_room(buff_temp);
}

void on_back_button_clicked()
{
    gtk_widget_hide(window_main);
    send_leave_room();
    gtk_widget_show(window_home);
}

void on_cancel_button_clicked()
{
    gtk_widget_hide(window_choose_room);
    gtk_widget_hide(window_wait);
    send_leave_room();
    gtk_widget_show(window_home);
}

void on_newgame_button_clicked()
{
    gtk_widget_hide(window_main);
    int x, y;
    for (y = 0; y <= 9; y++)
    {
        for (x = 0; x <= 9; x++)
        {
            cBoardLoc [x][y] = 'E';
        }
    }
}

void on_exit_button_clicked()
{
    send_disconnect();
    gtk_widget_destroy(GTK_WIDGET(window_home));
    gtk_main_quit();

    exit(1);
}

void on_set_button_clicked()
{
    const gchar *send_buffer = gtk_entry_get_text(GTK_ENTRY(entry_name));
    if(strcmp(send_buffer, "") != 0)
    {
        send_name(send_buffer);
        gtk_widget_hide(set_button);
        gtk_widget_hide(entry_name);
        label_name = gtk_label_new("");
        const char *format = "<span font=\"16\" color=\"red\" style=\"italic\">\%s</span>";
        char *markup;

        markup = g_markup_printf_escaped (format, send_buffer);
        gtk_label_set_markup (GTK_LABEL (label_name), markup);
        g_free (markup);
        gtk_fixed_put (GTK_FIXED (fixed_home), label_name, 200, 223);

        gtk_widget_show(label_name);
        gtk_widget_show (choose_room_button);
        gtk_widget_show (world_message_view);
        gtk_widget_show (world_msg_scrolling);
        gtk_widget_show (send_world_message_button);
        gtk_widget_show (world_entry_message);

    }
    else
    {
        Show_message(window_home, GTK_MESSAGE_ERROR, "ERROR", "Invalid name, try again!");
    }

}

void server_respond_choose_room_button(char *data)
{
    printf("test\n");
    gtk_widget_hide(window_home);
    init_choose_room_window(data);
}

void init_result_game_window()
{
    window_result = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (window_result), "delete_event", G_CALLBACK (delete_event), NULL);
    g_signal_connect (G_OBJECT (window_result), "destroy", G_CALLBACK (destroy), NULL);
    gtk_window_set_title (GTK_WINDOW (window_result), "RESULT");
    gtk_window_set_default_size(GTK_WINDOW(window_result), 350, 200);
    gtk_window_set_position(GTK_WINDOW(window_result), GTK_WIN_POS_CENTER);

    fixed_result = gtk_fixed_new ();
    gtk_container_add (GTK_CONTAINER (window_result), fixed_result);
    label_result = gtk_label_new("");

    ok_button = gtk_button_new_with_label ("OK");
    gtk_widget_set_size_request(ok_button, 70, 30);
    gtk_fixed_put (GTK_FIXED (fixed_result), ok_button, 150, 150);

    g_signal_connect(G_OBJECT(ok_button), "clicked", G_CALLBACK(on_ok_button_clicked), NULL);
}

void win_game ()
{
    const char *format = "<span font=\"15\" color=\"blue\" >\%s</span>";
    char *markup;

    markup = g_markup_printf_escaped (format, "You win!");
    gtk_label_set_markup (GTK_LABEL (label_result), markup);
    g_free (markup);
    gtk_fixed_put (GTK_FIXED (fixed_result), label_result, 140, 50);

    gtk_widget_show(label_result);
    gtk_widget_show(ok_button);
    gtk_widget_show (fixed_result);
    gtk_widget_show (window_result);
}

void lose_game ()
{
    const char *format = "<span font=\"15\" color=\"blue\" >\%s</span>";
    char *markup;

    markup = g_markup_printf_escaped (format, "You lose!");
    gtk_label_set_markup (GTK_LABEL (label_result), markup);
    g_free (markup);
    gtk_fixed_put (GTK_FIXED (fixed_result), label_result, 140, 50);

    gtk_widget_show(label_result);
    gtk_widget_show(ok_button);
    gtk_widget_show (fixed_result);
    gtk_widget_show (window_result);
}

void on_ok_button_clicked()
{
    gtk_widget_hide(window_main);
    gtk_widget_hide(window_result);
    send_leave_room();
    gtk_widget_show(window_home);
}

void on_close_button_clicked()
{
    gtk_widget_hide(window_choose_room);
    gtk_widget_hide(window_result);
    send_leave_room();
    gtk_widget_show(window_home);
}

void room_full_notice()
{
    const char *format = "<span font=\"15\" color=\"blue\" >\%s</span>";
    char *markup;

    markup = g_markup_printf_escaped (format, "Room is full!");
    gtk_label_set_markup (GTK_LABEL (label_result), markup);
    g_free (markup);
    gtk_fixed_put (GTK_FIXED (fixed_result), label_result, 125, 50);

    GtkWidget *close_button = gtk_button_new_with_label ("OK");
    gtk_widget_set_size_request(close_button, 70, 30);
    gtk_fixed_put (GTK_FIXED (fixed_result), close_button, 150, 150);
    g_signal_connect(G_OBJECT(close_button), "clicked", G_CALLBACK(on_close_button_clicked), NULL);

    gtk_widget_show(label_result);
    gtk_widget_show(close_button);
    gtk_widget_show (fixed_result);
    gtk_widget_show (window_result);
}