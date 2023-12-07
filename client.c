#include <stdio.h>
#include <string.h>
#include <gtk/gtkx.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 4463

GtkWindow *SelectRole;
GtkBuilder *builder;
GtkBuilder *welcome_builder;
GtkWindow *WelcomePage;
GtkWindow *SignUp;
GtkWindow *ManagerRegistration;
GtkWindow *IncorrectPassword;
GtkWindow *EmptyField;
GtkBuilder *sign_up_builder;
GtkWindow *Login;
GtkBuilder *login_builder;
GtkBuilder *main_builder;
GtkWindow *MAINPAGE;
GtkWindow *Hotels;
GtkBuilder *hotels_builder;

GtkComboBoxText *hotel_selector;

typedef struct {
    long long id;
    char name[255];
} HotelName;

typedef struct {
    char name[255];
    char address[255];
    char phone[20];
    char email[255];
    float rating;
    char facilities[255];
    char picture[1000];
} Hotel;

typedef struct {
    Hotel *hotels;
    int num_hotels;
} HotelList;



int sock; // Global variable for the socket
long long generateUniqueID() ;
void disconnect_from_server() ;
HotelList receive_hotels(int sock) ;
void receive_hotel_names(int sock, GtkComboBoxText *hotel_selector);


void connect_to_server() {
    struct sockaddr_in serv_addr;

    printf("CREATING CLIENT SOCKET .....\n");
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    printf("DEFINING CLIENT SOCKET FAMILY, ADDRESS & PORT .....\n");
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    printf("CLIENT CONNECTING TO SERVER.....\n");
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed. Retrying...");
        sleep(1); // Wait for a while before retrying
    }

    printf("Connection to server established\n");
}

void send_to_server(const char *data) {
    size_t data_len = strlen(data);

    if (send(sock, data, data_len, 0) != data_len) {
        fprintf(stderr, "Error sending data to server\n");
        // Handle the error, close the connection, or take appropriate action
    } else {
        printf("Data sent to server: %s\n", data);
    }
}



int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    // Builder for main.glade
    builder = gtk_builder_new_from_file("main.glade");
    SelectRole = GTK_WINDOW(gtk_builder_get_object(builder, "SelectRole"));

    welcome_builder=gtk_builder_new_from_file("page_1.glade");
    WelcomePage = GTK_WINDOW(gtk_builder_get_object(welcome_builder, "WelcomePage"));

    sign_up_builder=gtk_builder_new_from_file("page3.glade");
    SignUp = GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "SignUp"));
    ManagerRegistration = GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "ManagerRegistration"));
    IncorrectPassword=GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "IncorrectPassword"));
    EmptyField=GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "EmptyField"));

    login_builder=gtk_builder_new_from_file("page_2.glade");
    Login = GTK_WINDOW(gtk_builder_get_object(login_builder, "Login"));

    main_builder=gtk_builder_new_from_file("main_page/page_main.glade");
    MAINPAGE = GTK_WINDOW(gtk_builder_get_object(main_builder, "MAINPAGE"));

    hotels_builder=gtk_builder_new_from_file("hotels.glade");
    Hotels = GTK_WINDOW(gtk_builder_get_object(hotels_builder, "Hotels"));

    g_signal_connect(SelectRole, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(builder, NULL);

    g_signal_connect(WelcomePage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(welcome_builder, NULL);

    g_signal_connect(SignUp, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(sign_up_builder, NULL);

    g_signal_connect(ManagerRegistration, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(sign_up_builder, NULL);

    g_signal_connect(Login, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(login_builder, NULL);

    g_signal_connect(MAINPAGE, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(main_builder, NULL);

    g_signal_connect(Hotels, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(hotels_builder, NULL);


    connect_to_server();
    gtk_widget_show(GTK_WIDGET(SelectRole));
    gtk_main();
    


    return EXIT_SUCCESS;
}

void disconnect_from_server() {
    // Close the socket
    close(sock);
    printf("Disconnected from the server.\n");
}


//This is the buttons in Main.glade file 
void customer_button_clicked_cb (){
    gtk_widget_hide (GTK_WIDGET(SelectRole));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}


void admin_button_clicked_cb(){
    
}

//This is page_1.glade buttons
void welcome_register_clicked_cb() {
    gtk_widget_hide(GTK_WIDGET(WelcomePage));
    gtk_widget_show(GTK_WIDGET(SignUp));  
    
}
void receive_hotel_names(int sock, GtkComboBoxText *hotel_selector) {
    int num_names;
    recv(sock, &num_names, sizeof(num_names), 0);

    printf("Received %d hotel names from the server:\n", num_names);

    for (int i = 0; i < num_names; ++i) {
        HotelName hotelName;
        recv(sock, &hotelName, sizeof(HotelName), 0);

        // Convert hotel ID to string
        gchar *id_str = g_strdup_printf("%lld", hotelName.id);

        // Add hotel name to the combo box with the ID as the data
        gtk_combo_box_text_append(hotel_selector, id_str, hotelName.name);

        // Print the received data
        printf("Hotel %d:\n", i + 1);
        printf("ID: %lld\n", hotelName.id);
        printf("Name: %s\n", hotelName.name);
        printf("\n");

        g_free(id_str);
    }
}

        

void welcome_login_clicked_cb(){

    gtk_widget_hide (GTK_WIDGET(WelcomePage));
 	gtk_widget_show (GTK_WIDGET(Login));
}

void manager_button_clicked_cb(){
    send_to_server("NAMES| ");
    hotel_selector = GTK_COMBO_BOX_TEXT (gtk_builder_get_object (sign_up_builder, "hotel_selector"));
    gtk_combo_box_text_remove_all (hotel_selector);
    receive_hotel_names(sock,hotel_selector);
    gtk_widget_hide (GTK_WIDGET(SelectRole));
 	gtk_widget_show (GTK_WIDGET(ManagerRegistration));
}


void manager_register_button_clicked_cb(){
    GtkEntry *entry_firstname = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "first_name1"));
    GtkEntry *entry_lastname = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "last_name1"));
    GtkEntry *entry_address = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "address1"));
    GtkEntry *entry_passport_number = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "passport_number1"));
    GtkEntry *entry_email = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "email1"));
    GtkEntry *entry_phone_number = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "phone_number1"));
    GtkEntry *entry_username = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "username1"));
    GtkEntry *entry_password = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "password1"));
    GtkEntry *entry_confirm_password = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "confirm_password1"));

    // Extract values from the entry widgets
    const gchar *firstname = gtk_entry_get_text(entry_firstname);
    const gchar *lastname = gtk_entry_get_text(entry_lastname);
    const gchar *address = gtk_entry_get_text(entry_address);
    const gchar *passport_number = gtk_entry_get_text(entry_passport_number);
    const gchar *email = gtk_entry_get_text(entry_email);
    const gchar *phone_number = gtk_entry_get_text(entry_phone_number);
    const gchar *username = gtk_entry_get_text(entry_username);
    const gchar *password = gtk_entry_get_text(entry_password);
    const gchar *confirm_password = gtk_entry_get_text(entry_confirm_password);
    const gchar *selected_hotel_name = gtk_combo_box_text_get_active_text(hotel_selector);

    if (strlen(firstname) == 0 || strlen(lastname) == 0 || strlen(address) == 0 ||
        strlen(passport_number) == 0 || strlen(email) == 0 || strlen(phone_number) == 0 ||
        strlen(username) == 0 || strlen(password) == 0 || strlen(confirm_password) == 0) {
        gtk_widget_show(GTK_WIDGET(EmptyField));
        g_print("Please fill in all required fields\n");
        return;
    }
    if (strcmp(password, confirm_password) != 0) {
        gtk_widget_show(GTK_WIDGET(IncorrectPassword));
    } else {
        long long manager_id=generateUniqueID();
        char data[1024]; 
        snprintf(data, sizeof(data), "MANAGER_REGISTER|%lld|%s|%s|%s|%s|%s|%s|%s|%s|%s", manager_id,
             firstname, lastname, address, passport_number, email, phone_number, username, password,selected_hotel_name);    
        g_print("DATA: %s\n", data);
        send_to_server(data);
        gtk_widget_hide(GTK_WIDGET(ManagerRegistration));
        gtk_widget_show(GTK_WIDGET(Login));
    }
     

}

void register_button_clicked_cb(){
    // Initialize entry widgets
    GtkEntry *entry_firstname = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "first_name"));
    GtkEntry *entry_lastname = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "last_name"));
    GtkEntry *entry_address = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "address"));
    GtkEntry *entry_passport_number = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "passport_number"));
    GtkEntry *entry_email = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "email"));
    GtkEntry *entry_phone_number = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "phone_number"));
    GtkEntry *entry_username = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "username"));
    GtkEntry *entry_password = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "password"));
    GtkEntry *entry_confirm_password = GTK_ENTRY(gtk_builder_get_object(sign_up_builder, "confirm_password"));

    // Extract values from the entry widgets
    const gchar *firstname = gtk_entry_get_text(entry_firstname);
    const gchar *lastname = gtk_entry_get_text(entry_lastname);
    const gchar *address = gtk_entry_get_text(entry_address);
    const gchar *passport_number = gtk_entry_get_text(entry_passport_number);
    const gchar *email = gtk_entry_get_text(entry_email);
    const gchar *phone_number = gtk_entry_get_text(entry_phone_number);
    const gchar *username = gtk_entry_get_text(entry_username);
    const gchar *password = gtk_entry_get_text(entry_password);
    const gchar *confirm_password = gtk_entry_get_text(entry_confirm_password);

    // Check if any of the entry fields are empty
    if (strlen(firstname) == 0 || strlen(lastname) == 0 || strlen(address) == 0 ||
        strlen(passport_number) == 0 || strlen(email) == 0 || strlen(phone_number) == 0 ||
        strlen(username) == 0 || strlen(password) == 0 || strlen(confirm_password) == 0) {
        gtk_widget_show(GTK_WIDGET(EmptyField));
        g_print("Please fill in all required fields\n");
        return;
    }
     
    if (strcmp(password, confirm_password) != 0) {
        gtk_widget_show(GTK_WIDGET(IncorrectPassword));
    } else {
        long long customer_id=generateUniqueID();
        char data[1024]; 
        snprintf(data, sizeof(data), "CLIENT_REGISTER|%lld|%s|%s|%s|%s|%s|%s|%s|%s", customer_id,
             firstname, lastname, address, passport_number, email, phone_number, username, password);    
        g_print("DATA: %s\n", data);
        send_to_server(data);
        gtk_widget_hide(GTK_WIDGET(SignUp));
        gtk_widget_show(GTK_WIDGET(Login));
    }
}

void login_submit_clicked_cb(){
    GtkEntry *username_entry, *password_entry;
    username_entry = GTK_ENTRY(gtk_builder_get_object(login_builder, "login_username"));
    password_entry = GTK_ENTRY(gtk_builder_get_object(login_builder, "login_password"));
    const gchar *username = gtk_entry_get_text(username_entry);
    const gchar *password = gtk_entry_get_text(password_entry);
    char data[1024]; 

    snprintf(data, sizeof(data), "LOGIN|%s|%s", username, password);

    send_to_server(data);

    char response[1024];
    ssize_t received_bytes = recv(sock, response, sizeof(response), 0);

    if (received_bytes > 0) {
        response[received_bytes] = '\0'; // Null-terminate the received data
        printf("RESPONSE: [%s]\n", response);

        // Check the response and take appropriate action
        if (strcmp(response, "true") == 0) {
            printf("Login successful. Display main page.\n");
            // Add code to display the main page
            gtk_widget_hide(GTK_WIDGET(Login));
            gtk_widget_show(GTK_WIDGET(MAINPAGE));
        } else {
            printf("Login failed. Display error popup.\n");
            // Add code to display an error popup
        }
    } else if (received_bytes == 0) {
        printf("Connection closed by the server.\n");
        // Add code to handle the case where the server closes the connection
    } else {
        perror("Error receiving data from the server");
        // Add error handling code as needed
    }

}
//This is for generating CustomerId
long long generateUniqueID() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    // Convert timeval to milliseconds
    long long milliseconds = currentTime.tv_sec * 1000LL + currentTime.tv_usec / 1000LL;

    static int counter = 0;
    long long uniqueID = milliseconds  + counter;
    counter++;

    return uniqueID;
}


void incorrect_password_ok_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(IncorrectPassword));
}
void empty_field_ok_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(EmptyField));
}

void buxara_btn_clicked_cb(){
  
    send_to_server("HOTELS ");
    HotelList all_hotels=receive_hotels(sock);  
    //create_hotel_frames(all_hotels);
    
}

HotelList receive_hotels(int sock) {
    HotelList hotelList;

    // Receive the number of hotels
    recv(sock, &hotelList.num_hotels, sizeof(hotelList.num_hotels), 0);

    // Allocate memory for the hotels array
    hotelList.hotels = (Hotel *)malloc(hotelList.num_hotels * sizeof(Hotel));

    printf("Received %d hotels from the server:\n", hotelList.num_hotels);

    for (int i = 0; i < hotelList.num_hotels; ++i) {
        // Receive each hotel and store it in the array
        recv(sock, &hotelList.hotels[i], sizeof(Hotel), 0);

        // Process the received hotel data as needed
        printf("Hotel %d:\n", i + 1);
        printf("Name: %s\n", hotelList.hotels[i].name);
        printf("Address: %s\n", hotelList.hotels[i].address);
        printf("Phone: %s\n", hotelList.hotels[i].phone);
        printf("Email: %s\n", hotelList.hotels[i].email);
        printf("Rating: %.2f\n", hotelList.hotels[i].rating);
        printf("Facilities: %s\n", hotelList.hotels[i].facilities);
        printf("Picture: %s\n", hotelList.hotels[i].picture);
        printf("\n");
    }

    return hotelList;
}

// Function to free the memory allocated for a HotelList
void freeHotelList(HotelList *hotelList) {
    free(hotelList->hotels);
}