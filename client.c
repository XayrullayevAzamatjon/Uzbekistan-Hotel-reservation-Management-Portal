#include <stdio.h>
#include <string.h>
#include <gtk/gtkx.h>
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
GtkWindow *IncorrectPassword;
GtkWindow *EmptyField;
GtkBuilder *sign_up_builder;
GtkWindow *Login;
GtkBuilder *login_builder;

int sock; // Global variable for the socket
long long generateUniqueID() ;
void disconnect_from_server() ;


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
    IncorrectPassword=GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "IncorrectPassword"));
    EmptyField=GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "EmptyField"));

    login_builder=gtk_builder_new_from_file("page_2.glade");
    Login = GTK_WINDOW(gtk_builder_get_object(login_builder, "Login"));

    g_signal_connect(SelectRole, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(builder, NULL);

    g_signal_connect(WelcomePage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(welcome_builder, NULL);

    g_signal_connect(SignUp, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(sign_up_builder, NULL);

    g_signal_connect(Login, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(login_builder, NULL);


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

void manager_button_clicked_cb(){

}

void admin_button_clicked_cb(){
    
}

//This is page_1.glade buttons
void welcome_register_clicked_cb() {
    gtk_widget_hide(GTK_WIDGET(WelcomePage));
    gtk_widget_show(GTK_WIDGET(SignUp));  
}

        

void welcome_login_clicked_cb(){

    gtk_widget_hide (GTK_WIDGET(WelcomePage));
 	gtk_widget_show (GTK_WIDGET(Login));
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
        snprintf(data, sizeof(data), "REGISTER|%lld|%s|%s|%s|%s|%s|%s|%s|%s", customer_id,
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
            gtk_widget_show(GTK_WIDGET(WelcomePage));
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
    long long uniqueID = milliseconds * 1000 + counter;
    counter++;

    return uniqueID;
}


void incorrect_password_ok_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(IncorrectPassword));
}
void empty_field_ok_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(EmptyField));
}
