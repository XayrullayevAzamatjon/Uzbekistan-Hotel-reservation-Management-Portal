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

// #define SERVER_ADDRESS "172.18.0.29"
// #define SERVER_ADDRESS "172.18.0.29"
//#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_ADDRESS "172.18.0.29"
#define SERVER_PORT 4464

GtkBuilder *builder;
GtkWindow *SelectRole;

GtkBuilder *welcome_builder;
GtkWindow *WelcomePage;
GtkWindow *ManagerWelcome;

GtkBuilder *sign_up_builder;
GtkWindow *SignUp;
GtkWindow *ManagerRegistration;
GtkWindow *IncorrectPassword;
GtkWindow *EmptyField;

GtkBuilder *manager_register_builder;
GtkWindow *ManagerRegister;

GtkBuilder *login_builder;
GtkWindow *Login;
GtkWindow *ManagerLogin;
GtkWindow *AdminLogin;

GtkBuilder *main_builder;
GtkWindow *MAINPAGE;
GtkLabel *main_page_client_id;

GtkBuilder *update_builder;
GtkWindow *CustomerUpdate;
GtkWindow *ManagerUpdate;
GtkWindow *AdminUpdate;

GtkBuilder *customer_info_builder;
GtkWindow *CustomerInfo;
GtkWindow *ManagerInfo;
GtkWindow *AdminInfo;

GtkWindow *AdminPage1;
GtkBuilder *admin_page1_builder;

GtkWidget *main_window;
GtkComboBoxText *hotel_selector;

GtkWindow *MoreInfo;
GtkBuilder *more_info_builder;

GtkWindow *ManagerMainPage;
GtkBuilder *manager_mainPage_builder;

GtkWindow *AdminHotel;
GtkBuilder *admin_hotel_builder;

GtkWindow *AddHotel;
GtkBuilder *add_hotel_builder;

GtkWindow *BookingPage;
GtkBuilder *booking_builder;

GtkWindow *UpdateHotel;
GtkBuilder *update_hotel_builder;
GtkComboBoxText *hotels_combo;

GtkWindow *Payment;
GtkBuilder *payment_builder;

GtkWindow *Final;
GtkBuilder *final_builder;

typedef struct {
    long long id;
    char name[255];
} HotelName;

typedef struct {
    int hotelID;
    char name[255];
    char address[255];
    char phone[20];
    char email[255];
    float rating;
    char facilities[255];
    char picture[1000];
    char region[255]; 
} Hotel;


typedef struct {
    Hotel *hotels;
    int num_hotels;
} HotelList;
typedef struct {
    char customerID[255];
    char firstname[255];
    char lastname[255];
    char address[255];
    char passport_number[255];
    char email[255];
    char phone_number[15];
    char username[255];
    char password[255];
} User;

typedef struct {
    int num_users;
    User *users;
} UserList;

typedef struct {
    long long managerID;
    char firstname[255];
    char lastname[255];
    char address[255];
    char passport_number[20];
    char email[255];
    char phone_number[20];
    char username[255];
    char password[255];
    char hotel_name[255];
} Manager;

typedef struct {
    int num_managers;
    Manager *managers;
} ManagerList;


typedef struct {
    int roomID;
    int room_number;
    char room_type[255];
    char bed_type[255];
    int max_occupancy;
    float rate;
} Room;
typedef struct {
    int num_rooms;  // Number of available rooms
    Room *rooms;    // Array of available rooms
} RoomList;


int sock; // Global variable for the socket
long long customer_id;
long long manager_id;
long long admin_id;
char ch_from_text[255], ch_to_text[255];
char booked_hotel_name[255];
char ch_hotel_name[255];
void display_hotels(GtkWidget *widget, HotelList *hotel_list);
void display_users(GtkWidget *widget, UserList *user_list);
void display_managers(GtkWidget *widget, ManagerList *manager_list);
long long generateUniqueID() ;
void disconnect_from_server() ;
HotelList receive_hotels(int sock) ;
RoomList receive_available_rooms(int sock);
UserList receive_customers(int sock);
ManagerList receive_managers(int sock);
//New_code
void receive_hotel_names(int sock, GtkComboBoxText *hotel_selector,HotelList *hotel_list);
void destroy(GtkWidget *widget, gpointer data);
void on_book_button_clicked(GtkButton *button, gpointer data);
void setup_hotel_interface(HotelList *hotel_list);
void set_hotels_name_to_hotel_selector(const char *region);
void more_info_book_clicked_cb(GtkButton *button, gpointer user_data);
gdouble calculate_total_price(gdouble base_price, gboolean is_standart_checked, gboolean is_lux_checked, gboolean is_busines_checked,
                              gboolean is_king_checked, gboolean is_double_checked, gboolean is_single_checked, guint adult_count, guint child_count);

void admin_update_hotel_clicked_cb();
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
//Connect to server 
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
    
    final_builder = gtk_builder_new_from_file("final.glade");
    Final = GTK_WINDOW(gtk_builder_get_object(final_builder, "Final")); 
    
    payment_builder = gtk_builder_new_from_file("payment/payment.glade");
    Payment = GTK_WINDOW(gtk_builder_get_object(payment_builder, "Payment")); 
    
    welcome_builder=gtk_builder_new_from_file("page_1.glade");
    WelcomePage = GTK_WINDOW(gtk_builder_get_object(welcome_builder, "WelcomePage"));
    ManagerWelcome=GTK_WINDOW(gtk_builder_get_object(welcome_builder, "ManagerWelcome"));

    sign_up_builder=gtk_builder_new_from_file("page3.glade");
    SignUp = GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "SignUp"));    
    ManagerRegistration = GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "ManagerRegistration"));
    IncorrectPassword=GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "IncorrectPassword"));
    EmptyField=GTK_WINDOW(gtk_builder_get_object(sign_up_builder, "EmptyField"));
    
    manager_mainPage_builder=gtk_builder_new_from_file("manager_main.glade");
    ManagerMainPage=GTK_WINDOW(gtk_builder_get_object(manager_mainPage_builder, "ManagerMainPage"));

    manager_register_builder=gtk_builder_new_from_file("main_page/manager_register.glade");
    ManagerRegister=GTK_WINDOW(gtk_builder_get_object(manager_register_builder, "ManagerRegister"));
   
    login_builder=gtk_builder_new_from_file("login/login.glade");
    Login = GTK_WINDOW(gtk_builder_get_object(login_builder, "Login"));
    ManagerLogin=GTK_WINDOW(gtk_builder_get_object(login_builder, "ManagerLogin"));
    AdminLogin=GTK_WINDOW(gtk_builder_get_object(login_builder, "AdminLogin"));   

    main_builder=gtk_builder_new_from_file("main_page/page_main.glade");
    MAINPAGE = GTK_WINDOW(gtk_builder_get_object(main_builder, "MAINPAGE"));
    main_page_client_id = GTK_LABEL(gtk_builder_get_object(main_builder, "client_id"));

    customer_info_builder=gtk_builder_new_from_file("Customer_page.glade");
    CustomerInfo = GTK_WINDOW(gtk_builder_get_object(customer_info_builder, "CustomerInfo"));
    ManagerInfo=GTK_WINDOW(gtk_builder_get_object(customer_info_builder, "ManagerInfo"));
    AdminInfo=GTK_WINDOW(gtk_builder_get_object(customer_info_builder, "AdminInfo"));

    update_builder=gtk_builder_new_from_file("update/update page.glade");
    CustomerUpdate = GTK_WINDOW(gtk_builder_get_object(update_builder, "CustomerUpdate"));
    ManagerUpdate = GTK_WINDOW(gtk_builder_get_object(update_builder, "ManagerUpdate"));
    AdminUpdate = GTK_WINDOW(gtk_builder_get_object(update_builder, "AdminUpdate"));

    more_info_builder=gtk_builder_new_from_file("more_info/hotel_info.glade");
    MoreInfo = GTK_WINDOW(gtk_builder_get_object(more_info_builder, "MoreInfo"));

    admin_page1_builder=gtk_builder_new_from_file("admin/admin_page1.glade");
    AdminPage1 = GTK_WINDOW(gtk_builder_get_object(admin_page1_builder, "AdminPage1"));

    admin_hotel_builder=gtk_builder_new_from_file("admin/admin_hotel_page.glade");
    AdminHotel = GTK_WINDOW(gtk_builder_get_object(admin_hotel_builder, "AdminHotel"));

    add_hotel_builder=gtk_builder_new_from_file("admin/add_hotel.glade");
    AddHotel = GTK_WINDOW(gtk_builder_get_object(add_hotel_builder, "AddHotel"));

    booking_builder=gtk_builder_new_from_file("booking/booking_page.glade");
    BookingPage = GTK_WINDOW(gtk_builder_get_object(booking_builder, "BookingPage"));

    update_hotel_builder=gtk_builder_new_from_file("admin/update_hotel.glade");
    UpdateHotel = GTK_WINDOW(gtk_builder_get_object(update_hotel_builder, "UpdateHotel"));


    g_signal_connect(SelectRole, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(builder, NULL);

    g_signal_connect(Final, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(final_builder, NULL);
    
    g_signal_connect(WelcomePage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(welcome_builder, NULL);

    g_signal_connect(ManagerWelcome, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(welcome_builder, NULL);

    g_signal_connect(SignUp, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(sign_up_builder, NULL);

    g_signal_connect(ManagerRegistration, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(sign_up_builder, NULL);

    g_signal_connect(ManagerMainPage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(manager_mainPage_builder, NULL);

    g_signal_connect(ManagerRegister, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(manager_register_builder, NULL);

    g_signal_connect(Login, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ManagerLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(AdminLogin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(login_builder, NULL);

    g_signal_connect(MAINPAGE, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(main_builder, NULL);

    g_signal_connect(Payment, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(payment_builder, NULL);

    g_signal_connect(CustomerInfo, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ManagerInfo, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(AdminInfo, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(customer_info_builder, NULL);

    g_signal_connect(CustomerUpdate, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ManagerUpdate, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(AdminUpdate, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(update_builder, NULL);

    g_signal_connect(AdminPage1, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(admin_page1_builder, NULL);

    g_signal_connect(BookingPage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(booking_builder, NULL);

    g_signal_connect(UpdateHotel, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(update_hotel_builder, NULL);

    g_signal_connect(MoreInfo, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(more_info_builder, NULL);

    g_signal_connect(AdminHotel, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(admin_hotel_builder, NULL);

    g_signal_connect(AddHotel, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(add_hotel_builder, NULL);

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

void refresh_status_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(BookingPage));
 	gtk_widget_show (GTK_WIDGET(Payment));
}

//This is the buttons in Main.glade file 
void customer_button_clicked_cb (){
    gtk_widget_hide (GTK_WIDGET(SelectRole));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}

void main_page_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(Final));
 	gtk_widget_show (GTK_WIDGET(SelectRole));
}

void welcome_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(WelcomePage));
 	gtk_widget_show (GTK_WIDGET(SelectRole));
}

void log_in_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(Login));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}

void manager_log_in_clicked_cb (){
    gtk_widget_hide (GTK_WIDGET(ManagerWelcome));
 	gtk_widget_show (GTK_WIDGET(ManagerLogin));
}

void manager_log_in_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerLogin));
 	gtk_widget_show (GTK_WIDGET(ManagerWelcome));
}

void sign_up_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(SignUp));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}

void manager_reg_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerRegistration));
 	gtk_widget_show (GTK_WIDGET(SelectRole));
}

void update_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(CustomerUpdate));
 	gtk_widget_show (GTK_WIDGET(CustomerInfo));
}
void manager_update_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerUpdate));
 	gtk_widget_show (GTK_WIDGET(ManagerInfo));
}
void admin_update_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminUpdate));
 	gtk_widget_show (GTK_WIDGET(AdminInfo));
}
void cancel_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(CustomerUpdate));
 	gtk_widget_show (GTK_WIDGET(CustomerInfo));
}
void manager_cancel_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerUpdate));
 	gtk_widget_show (GTK_WIDGET(ManagerInfo));
}
void admin_cancel_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminUpdate));
 	gtk_widget_show (GTK_WIDGET(AdminInfo));
}
void customer_info_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(CustomerInfo));
 	gtk_widget_show (GTK_WIDGET(MAINPAGE));
}
void manager_info_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerInfo));
 	gtk_widget_show (GTK_WIDGET(ManagerMainPage));
}
void admin_info_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminInfo));
 	gtk_widget_show (GTK_WIDGET(AdminPage1));
}
void admin_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(SelectRole));
 	gtk_widget_show (GTK_WIDGET(AdminLogin));
}

void admin_log_in_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminLogin));
 	gtk_widget_show (GTK_WIDGET(SelectRole));
}

void manager_welcome_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(WelcomePage));
 	gtk_widget_show (GTK_WIDGET(SelectRole));
}

//This is page_1.glade buttons
void welcome_register_clicked_cb() {
    gtk_widget_hide(GTK_WIDGET(WelcomePage));
    gtk_widget_show(GTK_WIDGET(SignUp));  
    
}

//New_code
void receive_hotel_names(int sock, GtkComboBoxText *hotel_selector, HotelList *hotel_list) {

    for (int i = 0; i < hotel_list->num_hotels; i++) {
        gchar *id_str = g_strdup_printf("%s", hotel_list->hotels[i].name);

        // Use the hotel name as both ID and display text
        gtk_combo_box_text_append(hotel_selector, id_str, hotel_list->hotels[i].name);
        printf("Added: %s\n", hotel_list->hotels[i].name);

        g_free(id_str);
    }
}       

void welcome_login_clicked_cb(){
     gtk_widget_hide (GTK_WIDGET(WelcomePage));
 	gtk_widget_show (GTK_WIDGET(Login));
}

void manager_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(SelectRole));
 	gtk_widget_show (GTK_WIDGET(ManagerWelcome));
}

void manager_region_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerRegister));
 	gtk_widget_show (GTK_WIDGET(ManagerWelcome));
}
 
void manager_sign_up_back_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(SignUp));
 	gtk_widget_show (GTK_WIDGET(ManagerWelcome));
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
        manager_id=generateUniqueID();
        char data[1024]; 
        snprintf(data, sizeof(data), "MANAGER_REGISTER|%lld|%s|%s|%s|%s|%s|%s|%s|%s|%s", manager_id,
             firstname, lastname, address, passport_number, email, phone_number, username, password,selected_hotel_name);    
        g_print("DATA: %s\n", data);
        send_to_server(data);
        gtk_widget_hide(GTK_WIDGET(ManagerRegistration));
        gtk_widget_show(GTK_WIDGET(ManagerLogin));
    }
     

}
void manager_login_button_clicked_cb(){
    GtkEntry *username_entry, *password_entry;
    username_entry = GTK_ENTRY(gtk_builder_get_object(login_builder, "login_username1"));
    password_entry = GTK_ENTRY(gtk_builder_get_object(login_builder, "login_password1"));
    GtkLabel *label_manager  =     GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "manager_id"));
    const gchar *username = gtk_entry_get_text(username_entry);
    const gchar *password = gtk_entry_get_text(password_entry);
    char data[1024]; 

    snprintf(data, sizeof(data), "MANAGER_LOGIN|%s|%s", username, password);

    send_to_server(data);

    char response[1024];
    ssize_t received_bytes = recv(sock, response, sizeof(response), 0);

    if (received_bytes > 0) {
        response[received_bytes] = '\0'; // Null-terminate the received data
        printf("RESPONSE: [%s]\n", response);

        // Check the response and take appropriate action
        char *token = strtok(response, "|");
        if (token != NULL && strcmp(token, "true") == 0) {
            // Login successful. Extract client ID from the response
            token = strtok(NULL, "|");
            if (token != NULL) {
                manager_id = atoll(token);
                const gchar *manager_id_str = g_strdup_printf("%lld", manager_id);

                printf("Login successful for client ID: %lld\n", manager_id);
                gtk_label_set_text(label_manager, manager_id_str);

                gtk_widget_hide(GTK_WIDGET(ManagerLogin));              
                gtk_widget_show(GTK_WIDGET(ManagerMainPage));     

            } else {
                fprintf(stderr, "Invalid response format: %s\n", response);
            }
        } else {

            printf("Login failed. Display error popup.\n");
       
        }
    } else if (received_bytes == 0) {
        printf("Connection closed by the server.\n");
    } else {
        perror("Error receiving data from the server");
    }

}
void refresh_clicked_cb() {
    char buffer[1024];
    ssize_t received_bytes = recv(sock, buffer, sizeof(buffer), MSG_DONTWAIT);

    if (received_bytes > 0) {
        buffer[received_bytes] = '\0'; // Null-terminate the received data
        printf("[SERVER] %s\n", buffer);

        // Process the received data only if it matches the expected format
        char from_text[255], to_text[255], room_type_str[255], bed_type_str[255], hotel_name[255];
        unsigned int adult_value, child_value, baby_value;
        float total_price;
        long long customer_id;

        if (sscanf(buffer, "FOR_MANAGER_CONFIRMATION|%254[^|]|%254[^|]|%u|%u|%u|%f|%254[^|]|%254[^|]|%254[^|]|%lld",
               from_text, to_text, &adult_value, &child_value, &baby_value, &total_price,
               room_type_str, bed_type_str, hotel_name, &customer_id) == 10) {

            // Convert numeric values to strings
            const gchar *str_customer_id = g_strdup_printf("%lld", customer_id);
            const gchar *str_adult_value = g_strdup_printf("%u", adult_value);
            const gchar *str_child_value = g_strdup_printf("%u", child_value);
            const gchar *str_baby_value = g_strdup_printf("%u", baby_value);
            const gchar *str_total_price = g_strdup_printf("%.2f", total_price);
            strcpy(ch_hotel_name, hotel_name);
            strcpy(ch_from_text, from_text);
            strcpy(ch_to_text, to_text);
            

            // Update the labels
            GtkLabel *label_customer_id  = GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "customer_id"));
            GtkLabel *label_num_adults  =  GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "num_adults"));
            GtkLabel *label_num_childs  =  GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "num_childs"));
            GtkLabel *label_num_babies  =  GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "num_babies"));
            GtkLabel *label_from_date=     GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "from_date"));
            GtkLabel *label_to_date  =     GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "to_date"));
            GtkLabel *label_room_type  =   GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "room_type"));
            GtkLabel *label_bed_type  =    GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "bed_type"));
            GtkLabel *label_price  =       GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "price"));
            GtkLabel *label_hotel_name  =  GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "hotel_name"));
            GtkLabel *label_manager  =     GTK_LABEL(gtk_builder_get_object(manager_mainPage_builder, "manager_id"));

            const gchar *manager_id_str = g_strdup_printf("%lld", manager_id);
            gtk_label_set_text(label_manager, manager_id_str);            
            gtk_label_set_text(label_hotel_name, hotel_name);
            gtk_label_set_text(label_customer_id, str_customer_id);
            gtk_label_set_text(label_num_adults,  str_adult_value);
            gtk_label_set_text(label_num_childs,  str_child_value);
            gtk_label_set_text(label_num_babies,  str_baby_value);
            gtk_label_set_text(label_from_date, from_text);
            gtk_label_set_text(label_to_date, to_text);
            gtk_label_set_text(label_room_type, room_type_str);
            gtk_label_set_text(label_bed_type, bed_type_str);
            gtk_label_set_text(label_price, str_total_price);

        } else {
            fprintf(stderr, "Invalid FOR_MANAGER_CONFIRMATION data format: %s\n", buffer);
        }
    }    
}
void check_available_rooms_clicked_cb(){
    char data[1024];
    snprintf(data, sizeof(data), "HOTEL_NAME|%s",ch_hotel_name);
    send_to_server(data);
    printf("FROM [%s] -> TO[%s]\n",ch_from_text,ch_to_text);

    HotelList hotels = receive_hotels(sock);
    if (hotels.num_hotels == 1) {
        char check_room[1024];
        snprintf(check_room, sizeof(check_room), "CHECK_ROOM|%d|%s|%s",hotels.hotels[0].hotelID,ch_from_text,ch_to_text);
        printf("CHECKING HOTEL ID  %d\n", hotels.hotels[0].hotelID);
        send_to_server(check_room);
        receive_available_rooms(sock);
    } else {
        g_print("Hotel not found.\n");
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
        customer_id=generateUniqueID();
        char data[1024]; 
        snprintf(data, sizeof(data), "CLIENT_REGISTER|%lld|%s|%s|%s|%s|%s|%s|%s|%s", customer_id,
             firstname, lastname, address, passport_number, email, phone_number, username, password);    
        g_print("DATA: %s\n", data);
        send_to_server(data);
        gtk_widget_hide(GTK_WIDGET(SignUp));
        gtk_widget_show(GTK_WIDGET(Login));
    }
}

void login_submit_clicked_cb() {
    GtkEntry *username_entry, *password_entry;
    username_entry = GTK_ENTRY(gtk_builder_get_object(login_builder, "login_username"));
    password_entry = GTK_ENTRY(gtk_builder_get_object(login_builder, "login_password"));
    const gchar *username = gtk_entry_get_text(username_entry);
    const gchar *password = gtk_entry_get_text(password_entry);
    char data[1024];

    snprintf(data, sizeof(data), "CLIENT_LOGIN|%s|%s", username, password);

    send_to_server(data);

    char response[1024];
    ssize_t received_bytes = recv(sock, response, sizeof(response), 0);

    if (received_bytes > 0) {
        response[received_bytes] = '\0'; // Null-terminate the received data
        printf("RESPONSE: [%s]\n", response);

        // Check the response and take appropriate action
        char *token = strtok(response, "|");
        if (token != NULL && strcmp(token, "true") == 0) {
            // Login successful. Extract client ID from the response
            token = strtok(NULL, "|");
            if (token != NULL) {
                customer_id = atoll(token);
                const gchar *customer_id_str = g_strdup_printf("%lld", customer_id);

                printf("Login successful for client ID: %lld\n", customer_id);
                gtk_label_set_text(main_page_client_id, customer_id_str);

                // Add code to display the main page
                gtk_widget_hide(GTK_WIDGET(Login));
                gtk_widget_show(GTK_WIDGET(MAINPAGE));
            } else {
                fprintf(stderr, "Invalid response format: %s\n", response);
            }
        } else {

            printf("Login failed. Display error popup.\n");
       
        }
    } else if (received_bytes == 0) {
        printf("Connection closed by the server.\n");
        // Add code to handle the case where the server closes the connection
    } else {
        perror("Error receiving data from the server");
        // Add error handling code as needed
    }
}
void admin_login_button_clicked_cb(){
    GtkEntry *username_entry, *password_entry;
    username_entry = GTK_ENTRY(gtk_builder_get_object(login_builder, "login_username2"));
    password_entry = GTK_ENTRY(gtk_builder_get_object(login_builder, "login_password2"));
    const gchar *username = gtk_entry_get_text(username_entry);
    const gchar *password = gtk_entry_get_text(password_entry);
    char data[1024];

    snprintf(data, sizeof(data),"ADMIN_LOGIN|%s|%s", username, password);

    send_to_server(data);

    char response[1024];
    ssize_t received_bytes = recv(sock, response, sizeof(response), 0);

    if (received_bytes > 0) {
        response[received_bytes] = '\0'; // Null-terminate the received data
        printf("RESPONSE: [%s]\n", response);

        // Check the response and take appropriate action
        char *token = strtok(response, "|");
        if (token != NULL && strcmp(token, "true") == 0) {
            // Login successful. Extract client ID from the response
            token = strtok(NULL, "|");
            if (token != NULL) {
                admin_id = atoll(token);
                const gchar *admin_id_str = g_strdup_printf("%lld", admin_id);

                printf("Login successful for Admin ID: %lld\n", admin_id);
                gtk_label_set_text(main_page_client_id, admin_id_str);

                // Add code to display the main page
                gtk_widget_hide(GTK_WIDGET(AdminLogin));
                gtk_widget_show(GTK_WIDGET(AdminPage1));
            } else {
                fprintf(stderr, "Invalid response format: %s\n", response);
            }
        } else {

            printf("Login failed. Display error popup.\n");
       
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
void profile_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(MAINPAGE));
    gtk_widget_show (GTK_WIDGET(CustomerInfo));
    char message[256];
    snprintf(message, sizeof(message), "CLIENT_INFO|%lld", customer_id);
    send_to_server(message);
    //Labels
    GtkLabel *label_fname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "f_name"));
    GtkLabel *label_lname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "l_name"));
    GtkLabel *label_email = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "email"));
    GtkLabel *label_address = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "address"));
    GtkLabel *label_username = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "username"));
    GtkLabel *label_phone_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "phone_number"));
    GtkLabel *label_passport_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "passport_number"));

    //GTK ENTRY 
    GtkEntry *entry_firstname = GTK_ENTRY(gtk_builder_get_object(update_builder, "first_name2"));
    GtkEntry *entry_lastname = GTK_ENTRY(gtk_builder_get_object(update_builder, "last_name2"));
    GtkEntry *entry_address = GTK_ENTRY(gtk_builder_get_object(update_builder, "address2"));
    GtkEntry *entry_passport_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "passport_number2"));
    GtkEntry *entry_email = GTK_ENTRY(gtk_builder_get_object(update_builder, "email2"));
    GtkEntry *entry_phone_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "phone_number2"));
    GtkEntry *entry_username = GTK_ENTRY(gtk_builder_get_object(update_builder, "username2"));
    GtkEntry *entry_password = GTK_ENTRY(gtk_builder_get_object(update_builder ,"password2"));
    GtkEntry *entry_confirm_password = GTK_ENTRY(gtk_builder_get_object(update_builder, "confirm_password2"));
    
    char buffer[1024];
    ssize_t received_bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0';

        // Check if the received message is CUSTOMER_INFO
        if (strstr(buffer, "CUSTOMER_INFO|") != NULL) {
            // Parse and process the customer information
            long long id;
            char firstname[255], lastname[255], address[255], passport_number[255];
            char email[255], phone_number[20], username[255], password[255];

            // Extract customer information from the received message
            sscanf(buffer, "CUSTOMER_INFO|%lld|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
                   &id, firstname, lastname, address, passport_number,
                   email, phone_number, username, password);

            // Process the customer information (replace this with your processing logic)
            printf("Received customer information:\n");
            printf("ID: %lld\n", id);
            printf("Name: %s %s\n", firstname, lastname);
            printf("Address: %s\n", address);
            printf("Passport Number: %s\n", passport_number);
            printf("Email: %s\n", email);
            printf("Phone Number: %s\n", phone_number);
            printf("Username: %s\n", username);
            printf("Password: %s\n", password);
            // Update the UI with the received customer information
            gtk_label_set_text(label_fname, firstname);
            gtk_label_set_text(label_lname, lastname);
            gtk_label_set_text(label_email, email);
            gtk_label_set_text(label_address, address);
            gtk_label_set_text(label_username, username);
            gtk_label_set_text(label_phone_number, phone_number);
            gtk_label_set_text(label_passport_number, passport_number);

            //For updating customer info 
            gtk_entry_set_text(entry_firstname, firstname);
            gtk_entry_set_text(entry_lastname, lastname);
            gtk_entry_set_text(entry_email, email);
            gtk_entry_set_text(entry_address, address);
            gtk_entry_set_text(entry_username, username);
            gtk_entry_set_text(entry_phone_number, phone_number);
            gtk_entry_set_text(entry_passport_number, passport_number);
            gtk_entry_set_text(entry_password, password);
            gtk_entry_set_text(entry_confirm_password, password);
        }
    }
    else{
        printf("Error is occured while receiving data CUSTOMER INFO from server!");
    }
}
void customer_edit_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(CustomerInfo));
    gtk_widget_show (GTK_WIDGET(CustomerUpdate));
}
void update_button_clicked_cb(){

    GtkLabel *label_fname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "f_name"));
    GtkLabel *label_lname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "l_name"));
    GtkLabel *label_email = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "email"));
    GtkLabel *label_address = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "address"));
    GtkLabel *label_username = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "username"));
    GtkLabel *label_phone_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "phone_number"));
    GtkLabel *label_passport_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "passport_number"));

    // Get the entered customer information
    GtkEntry *entry_firstname = GTK_ENTRY(gtk_builder_get_object(update_builder, "first_name2"));
    GtkEntry *entry_lastname = GTK_ENTRY(gtk_builder_get_object(update_builder, "last_name2"));
    GtkEntry *entry_address = GTK_ENTRY(gtk_builder_get_object(update_builder, "address2"));
    GtkEntry *entry_passport_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "passport_number2"));
    GtkEntry *entry_email = GTK_ENTRY(gtk_builder_get_object(update_builder, "email2"));
    GtkEntry *entry_phone_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "phone_number2"));
    GtkEntry *entry_username = GTK_ENTRY(gtk_builder_get_object(update_builder, "username2"));
    GtkEntry *entry_password = GTK_ENTRY(gtk_builder_get_object(update_builder ,"password2"));
    GtkEntry *entry_confirm_password = GTK_ENTRY(gtk_builder_get_object(update_builder, "confirm_password2"));

    const gchar *firstname = gtk_entry_get_text(entry_firstname);
    const gchar *lastname = gtk_entry_get_text(entry_lastname);
    const gchar *address = gtk_entry_get_text(entry_address);
    const gchar *passport_number = gtk_entry_get_text(entry_passport_number);
    const gchar *email = gtk_entry_get_text(entry_email);
    const gchar *phone_number = gtk_entry_get_text(entry_phone_number);
    const gchar *username = gtk_entry_get_text(entry_username);
    const gchar *password = gtk_entry_get_text(entry_password);
    const gchar *confirm_password = gtk_entry_get_text(entry_confirm_password);

    if (strlen(firstname) == 0 || strlen(lastname) == 0 || strlen(address) == 0 ||
        strlen(passport_number) == 0 || strlen(email) == 0 || strlen(phone_number) == 0 ||
        strlen(username) == 0 || strlen(password) == 0 || strlen(confirm_password) == 0) {
        gtk_widget_show(GTK_WIDGET(EmptyField));
        g_print("Please fill in all required fields\n");
        return;
    }
     
    if (strcmp(password, confirm_password) != 0) {
        gtk_widget_show(GTK_WIDGET(IncorrectPassword));
    }
    else {
        char data[1024]; 
        snprintf(data, sizeof(data), "CLIENT_UPDATE|%lld|%s|%s|%s|%s|%s|%s|%s|%s", customer_id,
             firstname, lastname, address, passport_number, email, phone_number, username, password);    
        g_print("DATA: %s\n", data);
        send_to_server(data);
        gtk_widget_hide(GTK_WIDGET(CustomerUpdate));
        gtk_label_set_text(label_fname, firstname);
        gtk_label_set_text(label_lname, lastname);
        gtk_label_set_text(label_email, email);
        gtk_label_set_text(label_address, address);
        gtk_label_set_text(label_username, username);
        gtk_label_set_text(label_phone_number, phone_number);
        gtk_label_set_text(label_passport_number, passport_number);
        gtk_widget_show(GTK_WIDGET(CustomerInfo));
    }
}

// nodira
void manager_profile_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerMainPage));
    gtk_widget_show (GTK_WIDGET(ManagerInfo));
    char message[256];
    snprintf(message, sizeof(message), "MANAGER_INFO|%lld", manager_id);
    send_to_server(message);
    //Labels
    GtkLabel *label_manager_fname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_f_name"));
    GtkLabel *label_manager_lname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_l_name"));
    GtkLabel *label_manager_email = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_email"));
    GtkLabel *label_manager_address = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_address"));
    GtkLabel *label_manager_username = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_username"));
    GtkLabel *label_manager_phone_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_phone_number"));
    GtkLabel *label_manager_passport_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_passport_number"));

    //GTK ENTRY 
    GtkEntry *entry_manager_firstname = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_first_name2"));
    GtkEntry *entry_manager_lastname = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_last_name2"));
    GtkEntry *entry_manager_address = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_address2"));
    GtkEntry *entry_manager_passport_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_passport_number2"));
    GtkEntry *entry_manager_email = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_email2"));
    GtkEntry *entry_manager_phone_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_phone_number2"));
    GtkEntry *entry_manager_username = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_username2"));
    GtkEntry *entry_manager_password = GTK_ENTRY(gtk_builder_get_object(update_builder ,"manager_password2"));
    GtkEntry *entry_manager_confirm_password = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_confirm_password2"));
    
    char buffer[1024];
    ssize_t received_bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0';

        // Check if the received message is MANAGER_INFO
        if (strstr(buffer, "MANAGER_INFO|") != NULL) {
            // Parse and process the manager information
            long long id;
            char manager_firstname[255], manager_lastname[255], manager_address[255], manager_passport_number[255];
            char manager_email[255], manager_phone_number[20], manager_username[255], manager_password[255];

            // Extract manager information from the received message
            sscanf(buffer, "MANAGER_INFO|%lld|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
                   &id, manager_firstname, manager_lastname, manager_address, manager_passport_number,
                   manager_email, manager_phone_number, manager_username, manager_password);

            // Process the customer information (replace this with your processing logic)
            printf("Received manager information:\n");
            printf("ID: %lld\n", id);
            printf("Name: %s %s\n", manager_firstname, manager_lastname);
            printf("Address: %s\n", manager_address);
            printf("Passport Number: %s\n", manager_passport_number);
            printf("Email: %s\n", manager_email);
            printf("Phone Number: %s\n", manager_phone_number);
            printf("Username: %s\n", manager_username);
            printf("Password: %s\n", manager_password);
            // Update the UI with the received manager information
            gtk_label_set_text(label_manager_fname, manager_firstname);
            gtk_label_set_text(label_manager_lname, manager_lastname);
            gtk_label_set_text(label_manager_email, manager_email);
            gtk_label_set_text(label_manager_address, manager_address);
            gtk_label_set_text(label_manager_username, manager_username);
            gtk_label_set_text(label_manager_phone_number, manager_phone_number);
            gtk_label_set_text(label_manager_passport_number, manager_passport_number);

            //For updating manager info 
            gtk_entry_set_text(entry_manager_firstname, manager_firstname);
            gtk_entry_set_text(entry_manager_lastname, manager_lastname);
            gtk_entry_set_text(entry_manager_email, manager_email);
            gtk_entry_set_text(entry_manager_address, manager_address);
            gtk_entry_set_text(entry_manager_username, manager_username);
            gtk_entry_set_text(entry_manager_phone_number, manager_phone_number);
            gtk_entry_set_text(entry_manager_passport_number, manager_passport_number);
            gtk_entry_set_text(entry_manager_password, manager_password);
            gtk_entry_set_text(entry_manager_confirm_password, manager_password);
        }
    }
    else{
        printf("Error is occured while receiving data MANAGER INFO from server!");
    }
}
// nodira
void manager_edit_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerInfo));
    gtk_widget_show (GTK_WIDGET(ManagerUpdate));
}
// nodira
void manager_update_button_clicked_cb(){

    GtkLabel *label_manager_fname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_f_name"));
    GtkLabel *label_manager_lname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_l_name"));
    GtkLabel *label_manager_email = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_email"));
    GtkLabel *label_manager_address = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_address"));
    GtkLabel *label_manager_username = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_username"));
    GtkLabel *label_manager_phone_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_phone_number"));
    GtkLabel *label_manager_passport_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "manager_passport_number"));

    // Get the entered manager information
    GtkEntry *entry_manager_firstname = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_first_name2"));
    GtkEntry *entry_manager_lastname = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_last_name2"));
    GtkEntry *entry_manager_address = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_address2"));
    GtkEntry *entry_manager_passport_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_passport_number2"));
    GtkEntry *entry_manager_email = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_email2"));
    GtkEntry *entry_manager_phone_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_phone_number2"));
    GtkEntry *entry_manager_username = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_username2"));
    GtkEntry *entry_manager_password = GTK_ENTRY(gtk_builder_get_object(update_builder ,"manager_password2"));
    GtkEntry *entry_manager_confirm_password = GTK_ENTRY(gtk_builder_get_object(update_builder, "manager_confirm_password2"));

    const gchar *manager_firstname = gtk_entry_get_text(entry_manager_firstname);
    const gchar *manager_lastname = gtk_entry_get_text(entry_manager_lastname);
    const gchar *manager_address = gtk_entry_get_text(entry_manager_address);
    const gchar *manager_passport_number = gtk_entry_get_text(entry_manager_passport_number);
    const gchar *manager_email = gtk_entry_get_text(entry_manager_email);
    const gchar *manager_phone_number = gtk_entry_get_text(entry_manager_phone_number);
    const gchar *manager_username = gtk_entry_get_text(entry_manager_username);
    const gchar *manager_password = gtk_entry_get_text(entry_manager_password);
    const gchar *manager_confirm_password = gtk_entry_get_text(entry_manager_confirm_password);

    if (strlen(manager_firstname) == 0 || strlen(manager_lastname) == 0 || strlen(manager_address) == 0 ||
        strlen(manager_passport_number) == 0 || strlen(manager_email) == 0 || strlen(manager_phone_number) == 0 ||
        strlen(manager_username) == 0 || strlen(manager_password) == 0 || strlen(manager_confirm_password) == 0) {
        gtk_widget_show(GTK_WIDGET(EmptyField));
        g_print("Please fill in all required fields\n");
        return;
    }
     
    if (strcmp(manager_password, manager_confirm_password) != 0) {
        gtk_widget_show(GTK_WIDGET(IncorrectPassword));
    }
    else {
        char data[1024]; 
        snprintf(data, sizeof(data), "MANAGER_UPDATE|%lld|%s|%s|%s|%s|%s|%s|%s|%s", manager_id,
             manager_firstname, manager_lastname, manager_address, manager_passport_number, manager_email, manager_phone_number, manager_username, manager_password);    
        g_print("DATA: %s\n", data);
        send_to_server(data);
        gtk_widget_hide(GTK_WIDGET(ManagerUpdate));
        gtk_label_set_text(label_manager_fname, manager_firstname);
        gtk_label_set_text(label_manager_lname, manager_lastname);
        gtk_label_set_text(label_manager_email, manager_email);
        gtk_label_set_text(label_manager_address, manager_address);
        gtk_label_set_text(label_manager_username, manager_username);
        gtk_label_set_text(label_manager_phone_number, manager_phone_number);
        gtk_label_set_text(label_manager_passport_number, manager_passport_number);
        gtk_widget_show(GTK_WIDGET(ManagerInfo));
    }
}
// nodira
void admin_profile_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminPage1));
    gtk_widget_show (GTK_WIDGET(AdminInfo));
    char message[256];
    snprintf(message, sizeof(message), "ADMIN_INFO|%lld", admin_id);
    send_to_server(message);
    //Labels
    GtkLabel *label_admin_fname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_f_name"));
    GtkLabel *label_admin_lname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_l_name"));
    GtkLabel *label_admin_email = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_email"));
    GtkLabel *label_admin_username = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_username"));
    GtkLabel *label_admin_phone_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_phone_number"));

    //GTK ENTRY 
    GtkEntry *entry_admin_firstname = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_first_name2"));
    GtkEntry *entry_admin_lastname = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_last_name2"));
    GtkEntry *entry_admin_email = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_email2"));
    GtkEntry *entry_admin_phone_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_phone_number2"));
    GtkEntry *entry_admin_username = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_username2"));
    GtkEntry *entry_admin_password = GTK_ENTRY(gtk_builder_get_object(update_builder ,"admin_password2"));
    GtkEntry *entry_admin_confirm_password = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_confirm_password2"));
    
    char buffer[1024];
    ssize_t received_bytes = recv(sock, buffer, sizeof(buffer), 0);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0';
        // Check if the received message is ADMIN_INFO
        if (strstr(buffer, "ADMIN_INFO|") != NULL) {
            // Parse and process the admin information
            long long id;
            char admin_firstname[255], admin_lastname[255];
            char admin_email[255], admin_phone_number[20], admin_username[255], admin_password[255];

            // Extract admin information from the received message
            sscanf(buffer, "ADMIN_INFO|%lld|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
                   &id, admin_firstname, admin_lastname,admin_username, admin_password,
                   admin_email, admin_phone_number);

            // Process the admin information (replace this with your processing logic)
            printf("Received admin information:\n");
            printf("ID: %lld\n", id);
            printf("Name: %s %s\n", admin_firstname, admin_lastname);
            printf("Email: %s\n", admin_email);
            printf("Phone Number: %s\n", admin_phone_number);
            printf("Username: %s\n", admin_username);
            printf("Password: %s\n", admin_password);

            // Update the UI with the received admin information
            gtk_label_set_text(label_admin_fname, admin_firstname);
            gtk_label_set_text(label_admin_lname, admin_lastname);
            gtk_label_set_text(label_admin_email, admin_email);
            gtk_label_set_text(label_admin_username, admin_username);
            gtk_label_set_text(label_admin_phone_number, admin_phone_number);

            //For updating customer info 
            gtk_entry_set_text(entry_admin_firstname, admin_firstname);
            gtk_entry_set_text(entry_admin_lastname, admin_lastname);
            gtk_entry_set_text(entry_admin_email, admin_email);
            gtk_entry_set_text(entry_admin_username, admin_username);
            gtk_entry_set_text(entry_admin_phone_number, admin_phone_number);
            gtk_entry_set_text(entry_admin_password, admin_password);
            gtk_entry_set_text(entry_admin_confirm_password, admin_password);
        }
    }
    else{
        printf("Error is occured while receiving data ADMIN INFO from server!");
    }
}
// nodira
void admin_edit_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminInfo));
    gtk_widget_show (GTK_WIDGET(AdminUpdate));
}
// nodira
void admin_update_button_clicked_cb(){

    GtkLabel *label_admin_fname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_f_name"));
    GtkLabel *label_admin_lname = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_l_name"));
    GtkLabel *label_admin_email = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_email"));
    GtkLabel *label_admin_username = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_username"));
    GtkLabel *label_admin_phone_number = GTK_LABEL(gtk_builder_get_object(customer_info_builder, "admin_phone_number"));

    // Get the entered customer information
    GtkEntry *entry_admin_firstname = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_first_name2"));
    GtkEntry *entry_admin_lastname = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_last_name2"));
    GtkEntry *entry_admin_email = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_email2"));
    GtkEntry *entry_admin_phone_number = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_phone_number2"));
    GtkEntry *entry_admin_username = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_username2"));
    GtkEntry *entry_admin_password = GTK_ENTRY(gtk_builder_get_object(update_builder ,"admin_password2"));
    GtkEntry *entry_admin_confirm_password = GTK_ENTRY(gtk_builder_get_object(update_builder, "admin_confirm_password2"));

    const gchar *admin_firstname = gtk_entry_get_text(entry_admin_firstname);
    const gchar *admin_lastname = gtk_entry_get_text(entry_admin_lastname);
    const gchar *admin_email = gtk_entry_get_text(entry_admin_email);
    const gchar *admin_phone_number = gtk_entry_get_text(entry_admin_phone_number);
    const gchar *admin_username = gtk_entry_get_text(entry_admin_username);
    const gchar *admin_password = gtk_entry_get_text(entry_admin_password);
    const gchar *admin_confirm_password = gtk_entry_get_text(entry_admin_confirm_password);

    if (strlen(admin_firstname) == 0 || strlen(admin_lastname) == 0 || strlen(admin_email) == 0 || strlen(admin_phone_number) == 0 ||
        strlen(admin_username) == 0 || strlen(admin_password) == 0 || strlen(admin_confirm_password) == 0) {
        gtk_widget_show(GTK_WIDGET(EmptyField));
        g_print("Please fill in all required fields\n");
        return;
    }
     
    if (strcmp(admin_password, admin_confirm_password) != 0) {
        gtk_widget_show(GTK_WIDGET(IncorrectPassword));
    }
    else {
        char data[1024]; 
        snprintf(data, sizeof(data), "CLIENT_UPDATE|%lld|%s|%s|%s|%s|%s|%s", admin_id,
             admin_firstname, admin_lastname, admin_username, admin_password, admin_email, admin_phone_number);    
        g_print("DATA: %s\n", data);
        send_to_server(data);
        gtk_widget_hide(GTK_WIDGET(AdminUpdate));
        gtk_label_set_text(label_admin_fname, admin_firstname);
        gtk_label_set_text(label_admin_lname, admin_lastname);
        gtk_label_set_text(label_admin_email, admin_email);
        gtk_label_set_text(label_admin_username, admin_username);
        gtk_label_set_text(label_admin_phone_number, admin_phone_number);
        gtk_widget_show(GTK_WIDGET(AdminInfo));
    }
}
RoomList receive_available_rooms(int sock) {
    RoomList roomList;

    // Receive the number of available rooms
    recv(sock, &roomList.num_rooms, sizeof(roomList.num_rooms), 0);

    // Allocate memory for the rooms array
    roomList.rooms = (Room *)malloc(roomList.num_rooms * sizeof(Room));

    printf("Received %d available rooms from the server:\n", roomList.num_rooms);

    for (int i = 0; i < roomList.num_rooms; ++i) {
        // Receive each room and store it in the array
        recv(sock, &roomList.rooms[i], sizeof(Room), 0);

        // Process the received room data as needed
        printf("Room ID: %d\n", roomList.rooms[i].roomID);
        printf("Room Number: %d\n", roomList.rooms[i].room_number);
        printf("Room Type: %s\n", roomList.rooms[i].room_type);
        printf("Bed Type: %s\n", roomList.rooms[i].bed_type);
        printf("Max Occupancy: %d\n", roomList.rooms[i].max_occupancy);
        printf("Rate: %.2f\n", roomList.rooms[i].rate);
        printf("\n");
    }

    return roomList;
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
        printf("Hotel id %d:\n", hotelList.hotels[i].hotelID);
        printf("Name: %s\n", hotelList.hotels[i].name);
        printf("Address: %s\n", hotelList.hotels[i].address);
        printf("Phone: %s\n", hotelList.hotels[i].phone);
        printf("Email: %s\n", hotelList.hotels[i].email);
        printf("Rating: %.2f\n", hotelList.hotels[i].rating);
        printf("Facilities: %s\n", hotelList.hotels[i].facilities);
        printf("Picture: %s\n", hotelList.hotels[i].picture);
        printf("Region: %s\n", hotelList.hotels[i].region); // Print the region field
        printf("\n");
    }

    return hotelList;
}
UserList receive_customers(int sock) {
    UserList userList;

    // Receive the number of users
    recv(sock, &userList.num_users, sizeof(userList.num_users), 0);

    // Allocate memory for the users array
    userList.users = (User *)malloc(userList.num_users * sizeof(User));

    printf("Received %d customers from the server:\n", userList.num_users);

    for (int i = 0; i < userList.num_users; ++i) {
        // Receive each user and store it in the array
        recv(sock, &userList.users[i], sizeof(User), 0);

        // Process the received user data as needed
        printf("Customer ID: %s\n", userList.users[i].customerID);
        printf("First Name: %s\n", userList.users[i].firstname);
        printf("Last Name: %s\n", userList.users[i].lastname);
        printf("Address: %s\n", userList.users[i].address);
        printf("Passport Number: %s\n", userList.users[i].passport_number);
        printf("Email: %s\n", userList.users[i].email);
        printf("Phone Number: %s\n", userList.users[i].phone_number);
        printf("Username: %s\n", userList.users[i].username);
        printf("Password: %s\n", userList.users[i].password);
        printf("\n");
    }

    return userList;
}
ManagerList receive_managers(int sock) {
    ManagerList managerList;

    // Receive the number of managers
    recv(sock, &managerList.num_managers, sizeof(managerList.num_managers), 0);

    // Allocate memory for the managers array
    managerList.managers = (Manager *)malloc(managerList.num_managers * sizeof(Manager));

    printf("Received %d managers from the server:\n", managerList.num_managers);

    for (int i = 0; i < managerList.num_managers; ++i) {
        // Receive each manager and store it in the array
        recv(sock, &managerList.managers[i], sizeof(Manager), 0);

        // Process the received manager data as needed
        printf("Manager ID: %lld\n", managerList.managers[i].managerID);
        printf("First Name: %s\n", managerList.managers[i].firstname);
        printf("Last Name: %s\n", managerList.managers[i].lastname);
        printf("Address: %s\n", managerList.managers[i].address);
        printf("Passport Number: %s\n", managerList.managers[i].passport_number);
        printf("Email: %s\n", managerList.managers[i].email);
        printf("Phone Number: %s\n", managerList.managers[i].phone_number);
        printf("Username: %s\n", managerList.managers[i].username);
        printf("Password: %s\n", managerList.managers[i].password);
        printf("Hotel Name: %s\n", managerList.managers[i].hotel_name);
        printf("\n");
    }

    return managerList;
}

// Function to free the memory allocated for a HotelList
/* void freeHotelList(HotelList *hotelList) {
    free(hotelList->hotels);
} */

void setup_hotel_interface(HotelList *hotel_list) {

    gtk_widget_hide(GTK_WIDGET(MAINPAGE));
    // Create the main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Hotel Details");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

    // Create a scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Create a vertical box to hold the hotel details
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(scrolled_window), vbox);

    // Create GTK widgets for each hotel
    for (int i = 0; i < hotel_list->num_hotels; i++) {
        GtkWidget *hotel_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        GtkWidget *image = gtk_image_new_from_file(hotel_list->hotels[i].picture);
        GtkWidget *details_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

        GtkWidget *hotel_name_label = gtk_label_new(hotel_list->hotels[i].name);
        GtkWidget *hotel_address_label = gtk_label_new(hotel_list->hotels[i].address);
        GtkWidget *hotel_rating_label = gtk_label_new(g_strdup_printf("Rating: %.2f", hotel_list->hotels[i].rating));

        GtkWidget *book_button = gtk_button_new_with_label("Book");
        g_signal_connect(book_button, "clicked", G_CALLBACK(on_book_button_clicked), &(hotel_list->hotels[i]));

        // Apply CSS styling to create rounded borders, padding, and box shadow
        const gchar *css_data = ".hotel-container { border-radius: 10px; background-color: #fff; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
        GtkCssProvider *cssProvider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(cssProvider, css_data, -1, NULL);

        // Apply CSS styling to the hotel container
        GtkStyleContext *styleContext = gtk_widget_get_style_context(hotel_container);
        gtk_style_context_add_provider(styleContext, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

        // Pack labels and button into the details_vbox
        gtk_box_pack_start(GTK_BOX(details_vbox), hotel_name_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(details_vbox), hotel_address_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(details_vbox), hotel_rating_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(details_vbox), book_button, FALSE, FALSE, 0);

        // Set the horizontal alignment for each label to left-aligned
        gtk_label_set_xalign(GTK_LABEL(hotel_name_label), 0.0);
        gtk_label_set_xalign(GTK_LABEL(hotel_address_label), 0.0);
        gtk_label_set_xalign(GTK_LABEL(hotel_rating_label), 0.0);

        // Pack the hotel_container into the main vbox
        gtk_box_pack_start(GTK_BOX(hotel_container), image, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hotel_container), details_vbox, TRUE, TRUE, 0);

        gtk_box_pack_start(GTK_BOX(vbox), hotel_container, FALSE, FALSE, 0);
    }

    // Connect the destroy signal
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroy), NULL);

    // Pack the scrolled window into the main window
    gtk_container_add(GTK_CONTAINER(main_window), scrolled_window);

    // Show all widgets and start the main loop
    gtk_widget_show_all(main_window);
    gtk_main();
}

void on_book_button_clicked(GtkButton *button, gpointer data) {
    Hotel *hotel = (Hotel *)data;
    g_print("Book button clicked for hotel: %s\n", hotel->name);
    gtk_widget_hide(GTK_WIDGET(main_window));    
    GtkLabel *client_id = GTK_LABEL(gtk_builder_get_object(more_info_builder, "client_id"));
    GtkLabel *hotel_name = GTK_LABEL(gtk_builder_get_object(more_info_builder, "hotel_name"));
    GtkLabel *hotel_name1 = GTK_LABEL(gtk_builder_get_object(more_info_builder, "hotel_name1"));
    GtkLabel *address = GTK_LABEL(gtk_builder_get_object(more_info_builder, "address"));
    GtkLabel *rate = GTK_LABEL(gtk_builder_get_object(more_info_builder, "rate"));
    GtkLabel *hotel_features = GTK_LABEL(gtk_builder_get_object(more_info_builder, "hotel_features"));
    GtkImage *image1 = GTK_IMAGE(gtk_builder_get_object(more_info_builder, "hotel_image1"));
    GtkImage *image2 = GTK_IMAGE(gtk_builder_get_object(more_info_builder, "hotel_image2"));
    GtkImage *image3 = GTK_IMAGE(gtk_builder_get_object(more_info_builder, "hotel_image3"));
    strcpy(booked_hotel_name, hotel->name);
    // booked_hotel_name=hotel->name;

    gtk_image_set_from_file(image1, hotel->picture);
    gtk_image_set_from_file(image2, hotel->picture);
    gtk_image_set_from_file(image3, hotel->picture);
    const gchar *id_text = g_strdup_printf("%lld", customer_id);
    gtk_label_set_text(client_id, id_text);
    gtk_label_set_text(hotel_name, hotel->name);
    gtk_label_set_text(hotel_name1, hotel->name);
    gtk_label_set_text(address, hotel->address);
    const gchar *rating_text = g_strdup_printf("Rating: %.2f", hotel->rating);
    gtk_label_set_text(rate, rating_text);
    gtk_label_set_text(hotel_features, hotel->facilities);
    gtk_widget_show(GTK_WIDGET(MoreInfo));
    
    
}
void destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}
// //Booking page
void more_info_book_clicked_cb(GtkButton *button, gpointer user_data) {  
    g_print("Hotel name is [%s]",booked_hotel_name);
    gtk_widget_hide(GTK_WIDGET(MoreInfo));
    gtk_widget_show(GTK_WIDGET(BookingPage));
    GtkLabel *hotel_name_label = GTK_LABEL(gtk_builder_get_object(booking_builder, "hotel_name"));
    gtk_label_set_text(hotel_name_label, booked_hotel_name);
}
    gdouble calculate_total_price(gdouble base_price, gboolean is_standart_checked, gboolean is_lux_checked, gboolean is_busines_checked,
                              gboolean is_king_checked, gboolean is_double_checked, gboolean is_single_checked, guint adult_count, guint child_count) {
    gdouble total_price = base_price;
    if (is_standart_checked) {
        total_price += 10.0; 
    }
    if (is_lux_checked) {
        total_price += 20.0; 
    }
    if (is_busines_checked) {
        total_price += 40.0; 
    }
    if (is_king_checked) {
        total_price += 15.0; 
    }
    if (is_double_checked) {
        total_price += 10.0; 
    }
    if (is_single_checked) {
        total_price += 5.0; 
    }
    total_price += adult_count * 10.0; // $10 per adult
    total_price += child_count * 5.0;  // $5 per child

    return total_price;
}
void apply_button_clicked_cb(){
    g_print("Hotel name is [%s]",booked_hotel_name);
    char room_type_str[180] = ""; 
    char bed_type_str[180] = "";
// Retrieve the widgets
    GtkEntry *entry_from = GTK_ENTRY(gtk_builder_get_object(booking_builder, "from"));
    GtkEntry *entry_to = GTK_ENTRY(gtk_builder_get_object(booking_builder, "to"));

    GtkSpinButton *adult_spin_button = GTK_SPIN_BUTTON(gtk_builder_get_object(booking_builder, "adult_spinbutton"));
    GtkSpinButton *child_spinbutton = GTK_SPIN_BUTTON(gtk_builder_get_object(booking_builder, "child_spinbutton"));
    GtkSpinButton *baby_spinbutton = GTK_SPIN_BUTTON(gtk_builder_get_object(booking_builder, "baby_spinbutton"));

    GtkCheckButton *standart_check_button = GTK_CHECK_BUTTON(gtk_builder_get_object(booking_builder, "standart"));
    GtkCheckButton *lux_check_button = GTK_CHECK_BUTTON(gtk_builder_get_object(booking_builder, "lux"));
    GtkCheckButton *busines_check_button = GTK_CHECK_BUTTON(gtk_builder_get_object(booking_builder, "busines"));
    GtkCheckButton *king_check_button = GTK_CHECK_BUTTON(gtk_builder_get_object(booking_builder, "king"));
    GtkCheckButton *double_check_button = GTK_CHECK_BUTTON(gtk_builder_get_object(booking_builder, "double"));
    GtkCheckButton *single_check_button = GTK_CHECK_BUTTON(gtk_builder_get_object(booking_builder, "single"));

    // Get the values
    const gchar *from_text = gtk_entry_get_text(entry_from);
    const gchar *to_text = gtk_entry_get_text(entry_to);

    guint adult_value = gtk_spin_button_get_value_as_int(adult_spin_button);
    guint child_value = gtk_spin_button_get_value_as_int(child_spinbutton);
    guint baby_value = gtk_spin_button_get_value_as_int(baby_spinbutton);

    gboolean is_standart_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(standart_check_button));
    gboolean is_lux_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lux_check_button));
    gboolean is_busines_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(busines_check_button));
    gboolean is_king_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(king_check_button));
    gboolean is_double_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(double_check_button));
    gboolean is_single_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(single_check_button));

    if (is_standart_checked) {
        strncat(room_type_str, "Standard", sizeof(room_type_str) - strlen(room_type_str) - 1);
    }

    if (is_busines_checked) {        
        strncat(room_type_str, "Busines", sizeof(room_type_str) - strlen(room_type_str) - 1);
    }

    if (is_lux_checked) {        
        strncat(room_type_str, "Lux", sizeof(room_type_str) - strlen(room_type_str) - 1);
    }

    if (is_king_checked) {
        strcat(bed_type_str, "King");
    }

    if (is_double_checked) {
        strcat(bed_type_str, "Double");
    }

    if (is_single_checked) {
        strcat(bed_type_str, "Single");
    }


    // Assume a base price for the booking
    gdouble base_price = 50.0; 
    gdouble total_price = calculate_total_price(base_price, is_standart_checked, is_lux_checked, is_busines_checked,
                                                is_king_checked, is_double_checked, is_single_checked,adult_value,child_value);

    char data_to_send[1024];  // Adjust the size as needed
    snprintf(data_to_send, sizeof(data_to_send),
             "FOR_MANAGER_CONFIRMATION|%s|%s|%u|%u|%u|%.2f|%s|%s|%s|%lld",
             from_text, to_text, adult_value, child_value, baby_value, total_price,
             room_type_str, bed_type_str, booked_hotel_name, customer_id);
    send_to_server(data_to_send);
}

void pay_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(Payment));
 	gtk_widget_show (GTK_WIDGET(Final)); 
}

void manager_sign_up_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(ManagerWelcome));
 	gtk_widget_show (GTK_WIDGET(ManagerRegister));
}

void region_hotels(const char *region){
    char message[1024];
    snprintf(message, sizeof(message), "HOTELS|%s", region);
    send_to_server(message);
    HotelList hotels=receive_hotels(sock); 
    setup_hotel_interface(&hotels);
}

void m_buxara_clicked_cb(){
    set_hotels_name_to_hotel_selector("BUKHARA");
}
void m_tashkent_clicked_cb(){
    set_hotels_name_to_hotel_selector("TASHKENT");
}
void set_hotels_name_to_hotel_selector(const char *region){
    gtk_widget_hide (GTK_WIDGET(ManagerRegister));
 	gtk_widget_show (GTK_WIDGET(ManagerRegistration));
    char message[1024];
    snprintf(message, sizeof(message), "HOTELS|%s", region);
    send_to_server(message);
    HotelList hotels=receive_hotels(sock); 
    hotel_selector = GTK_COMBO_BOX_TEXT (gtk_builder_get_object (sign_up_builder, "hotel_selector"));
    gtk_combo_box_text_remove_all (hotel_selector);
    receive_hotel_names(sock,hotel_selector,&hotels);
}
//ADMIN PAGE 
void admin_hotel_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminPage1));
    gtk_widget_show (GTK_WIDGET(AdminHotel));
}
void admin_customer_clicked_cb(){
    send_to_server("ALL_USERS");   
    UserList users = receive_customers(sock);
    display_users(NULL, &users);
    free(users.users);
}

void admin_manager_clicked_cb(){
    send_to_server("ALL_MANAGERS");   
    ManagerList managers = receive_managers(sock);
    display_managers(NULL, &managers);
    free(managers.managers);

}
void admin_view_hotel_clicked_cb(){
    send_to_server("ALL_HOTELS");
    HotelList hotels = receive_hotels(sock);
    display_hotels(NULL, &hotels);
    free(hotels.hotels);
}
void admin_add_hotel_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminHotel));
    gtk_widget_show (GTK_WIDGET(AddHotel));
}

void add_hotel_button_clicked_cb(){

    GtkEntry *entry_hotel_name = GTK_ENTRY(gtk_builder_get_object(add_hotel_builder, "hotel_name"));
    GtkEntry *entry_address = GTK_ENTRY(gtk_builder_get_object(add_hotel_builder, "address"));
    GtkEntry *entry_email = GTK_ENTRY(gtk_builder_get_object(add_hotel_builder, "email"));
    GtkEntry *entry_phone = GTK_ENTRY(gtk_builder_get_object(add_hotel_builder, "phone"));
    GtkEntry *entry_rating = GTK_ENTRY(gtk_builder_get_object(add_hotel_builder, "rating"));
    GtkComboBoxText *region_selector = GTK_COMBO_BOX_TEXT (gtk_builder_get_object (add_hotel_builder, "region"));
    GtkEntry *entry_facilities = GTK_ENTRY(gtk_builder_get_object(add_hotel_builder, "facilities"));

    const gchar *hotel_name  = gtk_entry_get_text(entry_hotel_name);
    const gchar *address  = gtk_entry_get_text(entry_address);
    const gchar *email = gtk_entry_get_text(entry_email);
    const gchar *phone = gtk_entry_get_text(entry_phone);
    const gchar *rating = gtk_entry_get_text(entry_rating);        
    const gchar *selected_region = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(region_selector));
    const gchar *image = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gtk_builder_get_object(add_hotel_builder, "image")));
    const gchar *facilities  = gtk_entry_get_text(entry_facilities);
    
    // Find the last occurrence of the directory separator ("/")
    const char *lastSlash = strrchr(image, '/');

    // Extract the substring after the last slash
    const char *filename = (lastSlash != NULL) ? lastSlash + 1 : image;

    // Construct the new path in the "images" folder
    char newImagePath[256];  // Adjust the size as needed
    snprintf(newImagePath, sizeof(newImagePath), "images/%s", filename);

    g_print("Image Path: %s\n", newImagePath);

    char data_to_send[1024];  // Adjust the size as needed

    snprintf(data_to_send, sizeof(data_to_send), "ADD_HOTEL|%s|%s|%s|%s|%s|%s|%s|%s", hotel_name, address, email, phone, rating, selected_region, newImagePath, facilities);

    // Send the data to the server
    send_to_server(data_to_send);
    gtk_widget_hide (GTK_WIDGET(AddHotel));
    gtk_widget_show (GTK_WIDGET(AdminHotel));
}
void cancel_hotel_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AddHotel));
    gtk_widget_show (GTK_WIDGET(AdminHotel));
}
void add_hotel_back_button_clicked_cb(){
        gtk_widget_hide (GTK_WIDGET(AddHotel));
    gtk_widget_show (GTK_WIDGET(AdminHotel));
}

//Update Hotel
void admin_update_hotel_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(AdminHotel));
    send_to_server("ALL_HOTELS");

    HotelList hotels=receive_hotels(sock); 
    
    hotels_combo= GTK_COMBO_BOX_TEXT(gtk_builder_get_object(update_hotel_builder, "hotels"));
    gtk_combo_box_text_remove_all(hotels_combo);
    
    for (int i = 0; i < hotels.num_hotels; ++i) {
    gtk_combo_box_text_append_text(hotels_combo, hotels.hotels[i].name);
    }
    gtk_widget_show (GTK_WIDGET(UpdateHotel));

}
void update_select_clicked_cb() {
    GtkEntry *entry_hotel_name = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "name"));
    GtkEntry *entry_address = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "address"));
    GtkEntry *entry_email = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "email"));
    GtkEntry *entry_phone = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "phone"));
    GtkEntry *entry_facilities = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "facilities"));

    const gchar *selected_name = gtk_combo_box_text_get_active_text(hotels_combo);
    if (selected_name == NULL) {
        g_print("Error: No hotel selected.\n");
        return;
    }
    char data[1024];  // Adjust the size as needed

    snprintf(data, sizeof(data), "HOTEL_NAME|%s", selected_name);
    send_to_server(data);
    g_print("Selected Name: %s\n", selected_name);

    // Attach selected_name to the select button
    GtkButton *update_select_button = GTK_BUTTON(gtk_builder_get_object(update_hotel_builder, "update_select"));
    g_object_set_data(G_OBJECT(update_select_button), "selected_name", (gpointer)selected_name);

    HotelList hotels = receive_hotels(sock);
    // If the hotel data is received, set it to the entries
    if (hotels.num_hotels == 1) {
        gtk_entry_set_text(GTK_ENTRY(entry_hotel_name), hotels.hotels[0].name);
        gtk_entry_set_text(GTK_ENTRY(entry_address), hotels.hotels[0].address);
        gtk_entry_set_text(GTK_ENTRY(entry_email), hotels.hotels[0].email);
        gtk_entry_set_text(GTK_ENTRY(entry_phone), hotels.hotels[0].phone);
        gtk_entry_set_text(GTK_ENTRY(entry_facilities), hotels.hotels[0].facilities);
    } else {
        g_print("Hotel not found.\n");
    }

    //g_free((gchar *)selected_name);
}

void update_hotel_button_clicked_cb() {
    // Retrieve selected_name from the select button
    GtkButton *update_select_button = GTK_BUTTON(gtk_builder_get_object(update_hotel_builder, "update_select"));
    const gchar *selected_name = g_object_get_data(G_OBJECT(update_select_button), "selected_name");
    g_print("SELECTED [%s]",selected_name);
    if (selected_name == NULL) {
        g_print("Error: No hotel selected.\n");
        return;
    }

    GtkEntry *entry_hotel_name = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "name"));
    GtkEntry *entry_address = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "address"));
    GtkEntry *entry_email = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "email"));
    GtkEntry *entry_phone = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "phone"));
    GtkEntry *entry_facilities = GTK_ENTRY(gtk_builder_get_object(update_hotel_builder, "facilities"));

    const gchar *hotel_name = gtk_entry_get_text(GTK_ENTRY(entry_hotel_name));
    const gchar *address = gtk_entry_get_text(GTK_ENTRY(entry_address));
    const gchar *email = gtk_entry_get_text(GTK_ENTRY(entry_email));
    const gchar *phone = gtk_entry_get_text(GTK_ENTRY(entry_phone));
    const gchar *facilities = gtk_entry_get_text(GTK_ENTRY(entry_facilities));

        gtk_entry_set_text(GTK_ENTRY(entry_hotel_name), hotel_name);
        gtk_entry_set_text(GTK_ENTRY(entry_address), address);
        gtk_entry_set_text(GTK_ENTRY(entry_email),email );
        gtk_entry_set_text(GTK_ENTRY(entry_phone), phone);
        gtk_entry_set_text(GTK_ENTRY(entry_facilities),facilities );

    char server_data[1024];  // Adjust the size as needed
    snprintf(server_data, sizeof(server_data), "UPDATE_HOTEL|%s|%s|%s|%s|%s|%s",
             selected_name, hotel_name, address, email, phone, facilities);
    send_to_server(server_data);

    gtk_widget_hide(GTK_WIDGET(UpdateHotel));
    gtk_widget_show(GTK_WIDGET(AdminHotel));
}


void update_hotel_back_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(UpdateHotel));
    gtk_widget_show (GTK_WIDGET(AdminHotel));
}
void cancel_hotel_upate_button_clicked_cb(){
    gtk_widget_hide (GTK_WIDGET(UpdateHotel));
    gtk_widget_show (GTK_WIDGET(AdminHotel));  
}

//Main page buttons
void buxara_btn_clicked_cb(){
    region_hotels("BUKHARA");
}
void karakalpak_btn_clicked_cb(){
    region_hotels("KARAKALPAKSTAN");
}
void m_karakalpak_clicked_cb(){
    set_hotels_name_to_hotel_selector("KARAKALPAK");
}
void khorezm_btn_clicked_cb(){
    region_hotels("KHIVA");
}
void m_khorezm_clicked_cb(){
    set_hotels_name_to_hotel_selector("KHOREZM");
}
void kashkadarya_btn_clicked_cb(){
    region_hotels("KASHKADARYA");
}
void m_kashkadarya_clicked_cb(){
    set_hotels_name_to_hotel_selector("KASHKADARYA");
}
void surxan_btn_clicked_cb(){
    region_hotels("SURKHANDARYA");
}
void m_surxan_clicked_cb(){
    set_hotels_name_to_hotel_selector("SURXAN");
}
void jizzax_btn_clicked_cb(){
    region_hotels("JIZZAKH");
}
void m_jizzax_clicked_cb(){
    set_hotels_name_to_hotel_selector("JIZZAX");
}
void samarkand_btn_clicked_cb(){
    region_hotels("SAMARKAND");
}
void m_samarkand_clicked_cb(){
    set_hotels_name_to_hotel_selector("SAMARKAND");
}
void fergana_btn_clicked_cb(){
    region_hotels("FERGHANA");
}
void m_fergana_clicked_cb(){
    set_hotels_name_to_hotel_selector("FERGANA");
}
void namangan_btn_clicked_cb(){
    region_hotels("NAMANGAN");
}
void m_namangan_clicked_cb(){
    set_hotels_name_to_hotel_selector("NAMANGAN");
}
void tashkent_btn_clicked_cb(){
    region_hotels("TASHKENT");
}
void registan_clicked_cb(){
    region_hotels("SAMARKAND");
}
void historical_ark_clicked_cb(){
    region_hotels("BUKHARA");
}
void zamin_clicked_cb(){
    region_hotels("BUKHARA");
}


void display_hotels(GtkWidget *widget, HotelList *hotel_list) {
    GtkWidget *window, *vbox, *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkWidget *scroll;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_maximize(GTK_WINDOW(window));
    gtk_window_set_title(GTK_WINDOW(window), "Hotel List");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a list store to hold the data for the tree view
    store = gtk_list_store_new(9, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_STRING, G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    // Add data to the list store
    for (int i = 0; i < hotel_list->num_hotels; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, hotel_list->hotels[i].hotelID,
                           1, hotel_list->hotels[i].name,
                           2, hotel_list->hotels[i].address,
                           3, hotel_list->hotels[i].phone,
                           4, hotel_list->hotels[i].email,
                           5, hotel_list->hotels[i].rating,
                           6, hotel_list->hotels[i].facilities,
                           7, hotel_list->hotels[i].picture,
                           8, hotel_list->hotels[i].region,
                           -1);
    }

    // Create tree view
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_widget_set_vexpand(treeview, TRUE);

    // Add columns to the tree view
    renderer = gtk_cell_renderer_text_new();
    const char *field_names[] = {"HotelID", "Name", "Address", "Phone", "Email", "Rating", "Facilities", "Picture", "Region"};
    for (int i = 0; i < 9; i++) {
        GtkTreeViewColumn *column;
        if (i == 7) { // Assuming the "Picture" column is at index 7
            column = gtk_tree_view_column_new_with_attributes(
                field_names[i], renderer, "text", i, NULL);
        } else {
            column = gtk_tree_view_column_new_with_attributes(
                field_names[i], renderer, "text", i, NULL);
        }

        gtk_tree_view_column_set_title(column, field_names[i]);  // Set column title
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    }

    // Add tree view to scrollable window
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), treeview);

    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();
}
void display_users(GtkWidget *widget, UserList *user_list) {
    GtkWidget *window, *vbox, *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkWidget *scroll;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_maximize(GTK_WINDOW(window));
    gtk_window_set_title(GTK_WINDOW(window), "User List");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a list store to hold the data for the tree view
    store = gtk_list_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    // Add data to the list store
    for (int i = 0; i < user_list->num_users; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, user_list->users[i].customerID,
                           1, user_list->users[i].firstname,
                           2, user_list->users[i].lastname,
                           3, user_list->users[i].address,
                           4, user_list->users[i].passport_number,
                           5, user_list->users[i].email,
                           6, user_list->users[i].phone_number,
                           7, user_list->users[i].username,
                           8, user_list->users[i].password,
                           -1);
    }

    // Create tree view
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_widget_set_vexpand(treeview, TRUE);

    // Add columns to the tree view
    renderer = gtk_cell_renderer_text_new();
    const char *field_names[] = {"CustomerID", "Firstname", "Lastname", "Address", "Passport Number",
                                 "Email", "Phone Number", "Username", "Password"};
    for (int i = 0; i < 9; i++) {
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            field_names[i], renderer, "text", i, NULL);

        gtk_tree_view_column_set_title(column, field_names[i]);  // Set column title
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    }

    // Add tree view to scrollable window
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), treeview);

    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();
}
void display_managers(GtkWidget *widget, ManagerList *manager_list) {
    GtkWidget *window, *vbox, *treeview;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkWidget *scroll;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_maximize(GTK_WINDOW(window));
    gtk_window_set_title(GTK_WINDOW(window), "Manager List");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a list store to hold the data for the tree view
    store = gtk_list_store_new(10, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    // Add data to the list store
    for (int i = 0; i < manager_list->num_managers; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, manager_list->managers[i].managerID,
                           1, manager_list->managers[i].firstname,
                           2, manager_list->managers[i].lastname,
                           3, manager_list->managers[i].address,
                           4, manager_list->managers[i].passport_number,
                           5, manager_list->managers[i].email,
                           6, manager_list->managers[i].phone_number,
                           7, manager_list->managers[i].username,
                           8, manager_list->managers[i].password,
                           9, manager_list->managers[i].hotel_name,
                           -1);
    }

    // Create tree view
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_widget_set_vexpand(treeview, TRUE);

    // Add columns to the tree view
    renderer = gtk_cell_renderer_text_new();
    const char *field_names[] = {"ManagerID", "Firstname", "Lastname", "Address", "Passport Number",
                                 "Email", "Phone Number", "Username", "Password", "Hotel Name"};
    for (int i = 0; i < 10; i++) {
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            field_names[i], renderer, "text", i, NULL);

        gtk_tree_view_column_set_title(column, field_names[i]);  // Set column title
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    }

    // Add tree view to scrollable window
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), treeview);

    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();
}



