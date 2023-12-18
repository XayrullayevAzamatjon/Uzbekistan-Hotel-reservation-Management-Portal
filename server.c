#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 4465
void *handle_connection(void *arg);
void handle_client_login(const char *data, int client_fd);
void handle_manager_login(const char *data, int client_fd);
void handle_client_registration(const char *data);
void handle_manager_registration(const char *data);
void send_customer_info_to_server(long long customer_id, int client_fd);
void send_manager_info_to_server(long long manager_id, int client_fd);
void send_admin_info_to_server(long long admin_id, int client_fd);
MYSQL *connect_to_DB() ;
void handle_client_update(const char *data) ;
void handle_manager_update(const char *data);
void handle_admin_update(const char *data);
void handle_admin_login(const char *data, int client_fd);
void handle_add_hotel(const char *data);
int manager_fd;

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

void retrieve_all_customers(int client_fd) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM customers");

    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {

        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {

            int num_rows = mysql_num_rows(result);

            send(client_fd, &num_rows, sizeof(num_rows), 0);

            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != NULL) {

                User customer;
                snprintf(customer.customerID, sizeof(customer.customerID), "%s", row[0]); // Copy customerID
                snprintf(customer.firstname, sizeof(customer.firstname), "%s", row[1]); // Copy firstname
                snprintf(customer.lastname, sizeof(customer.lastname), "%s", row[2]);   // Copy lastname
                snprintf(customer.address, sizeof(customer.address), "%s", row[3]);     // Copy address
                snprintf(customer.passport_number, sizeof(customer.passport_number), "%s", row[4]); // Copy passport_number
                snprintf(customer.email, sizeof(customer.email), "%s", row[5]);         // Copy email
                snprintf(customer.phone_number, sizeof(customer.phone_number), "%s", row[6]);   // Copy phone_number
                snprintf(customer.username, sizeof(customer.username), "%s", row[7]);         // Copy username
                snprintf(customer.password, sizeof(customer.password), "%s", row[8]);         // Copy password

                send(client_fd, &customer, sizeof(User), 0);
            }

            mysql_free_result(result);
        }
    }

    mysql_close(conn);
}
void retrieve_all_managers(int client_fd) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM managers");

    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {

        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {

            int num_rows = mysql_num_rows(result);

            send(client_fd, &num_rows, sizeof(num_rows), 0);

            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != NULL) {

                Manager manager;
                manager.managerID = atoi(row[0]);                             // Convert and copy managerID
                snprintf(manager.firstname, sizeof(manager.firstname), "%s", row[1]); // Copy firstname
                snprintf(manager.lastname, sizeof(manager.lastname), "%s", row[2]);   // Copy lastname
                snprintf(manager.address, sizeof(manager.address), "%s", row[3]);     // Copy address
                snprintf(manager.passport_number, sizeof(manager.passport_number), "%s", row[4]); // Copy passport_number
                snprintf(manager.email, sizeof(manager.email), "%s", row[5]);         // Copy email
                snprintf(manager.phone_number, sizeof(manager.phone_number), "%s", row[6]);   // Copy phone_number
                snprintf(manager.username, sizeof(manager.username), "%s", row[7]);         // Copy username
                snprintf(manager.password, sizeof(manager.password), "%s", row[8]);         // Copy password
                snprintf(manager.hotel_name, sizeof(manager.hotel_name), "%s", row[9]);     // Copy hotel_name

                send(client_fd, &manager, sizeof(Manager), 0);
            }

            mysql_free_result(result);
        }
    }

    mysql_close(conn);
}



void retrieve_one_hotel(int client_fd, const char *hotel_name) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM hotels WHERE name='%s'", hotel_name);

    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {

        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {

            int num_rows = mysql_num_rows(result);

            send(client_fd, &num_rows, sizeof(num_rows), 0);

            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != NULL) {

                Hotel hotel;

                // Adjust the indices based on your new Hotel struct
                hotel.hotelID = atoi(row[0]);                                 // Convert and copy hotelID
                snprintf(hotel.name, sizeof(hotel.name), "%s", row[1]);       // Copy name
                snprintf(hotel.address, sizeof(hotel.address), "%s", row[2]); // Copy address
                snprintf(hotel.phone, sizeof(hotel.phone), "%s", row[3]);     // Copy phone
                snprintf(hotel.email, sizeof(hotel.email), "%s", row[4]);     // Copy email
                hotel.rating = strtof(row[5], NULL);                          // Convert and copy rating
                snprintf(hotel.facilities, sizeof(hotel.facilities), "%s", row[6]);  // Copy facilities
                snprintf(hotel.picture, sizeof(hotel.picture), "%s", row[7]);      // Copy picture
                snprintf(hotel.region, sizeof(hotel.region), "%s", row[8]);        // Copy region

                send(client_fd, &hotel, sizeof(Hotel), 0);
            }

            mysql_free_result(result);
        }
    }

    mysql_close(conn);
}

void retrieve_all_hotels(int client_fd) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM hotels");

    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {

        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {

            int num_rows = mysql_num_rows(result);

            send(client_fd, &num_rows, sizeof(num_rows), 0);

            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != NULL) {

                Hotel hotel;
                hotel.hotelID = atoi(row[0]);                                 // Convert and copy hotelID
                snprintf(hotel.name, sizeof(hotel.name), "%s", row[1]);       // Copy name
                snprintf(hotel.address, sizeof(hotel.address), "%s", row[2]); // Copy address
                snprintf(hotel.phone, sizeof(hotel.phone), "%s", row[3]);     // Copy phone
                snprintf(hotel.email, sizeof(hotel.email), "%s", row[4]);     // Copy email
                hotel.rating = strtof(row[5], NULL);                          // Convert and copy rating
                snprintf(hotel.facilities, sizeof(hotel.facilities), "%s", row[6]);  // Copy facilities
                snprintf(hotel.picture, sizeof(hotel.picture), "%s", row[7]);      // Copy picture
                snprintf(hotel.region, sizeof(hotel.region), "%s", row[8]);        // Copy region

                send(client_fd, &hotel, sizeof(Hotel), 0);
            }

            mysql_free_result(result);
        }
    }

    mysql_close(conn);
}

void retrieve_hotels(int client_fd, const char *region_name) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM hotels WHERE region='%s'", region_name);

    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {
            int num_rows = mysql_num_rows(result);
            send(client_fd, &num_rows, sizeof(num_rows), 0);

            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != NULL) {
                Hotel hotel;
                hotel.hotelID = atoi(row[0]); // Convert and copy hotelID
                snprintf(hotel.name, sizeof(hotel.name), "%s", row[1]);
                snprintf(hotel.address, sizeof(hotel.address), "%s", row[2]);
                snprintf(hotel.phone, sizeof(hotel.phone), "%s", row[3]);
                snprintf(hotel.email, sizeof(hotel.email), "%s", row[4]);
                hotel.rating = strtof(row[5], NULL);
                snprintf(hotel.facilities, sizeof(hotel.facilities), "%s", row[6]);
                snprintf(hotel.picture, sizeof(hotel.picture), "%s", row[7]);
                snprintf(hotel.region, sizeof(hotel.region), "%s", row[8]); // Copy region

                // Add more fields as needed

                send(client_fd, &hotel, sizeof(Hotel), 0);
            }

            mysql_free_result(result);
        }
    }

    mysql_close(conn);
}

void handle_hotel_update(const char *data) {
    char selected_name[1024], hotel_name[1024], address[1024], phone[15], email[1024], facilities[1024];


    if (sscanf(data, "UPDATE_HOTEL|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
            selected_name, hotel_name, address, email, phone, facilities) != 6) {
        fprintf(stderr, "Invalid hotel update data format: %s\n", data);
        return;
    }

    // Print the extracted values for debugging
    printf("Selected Name: %s\n", selected_name);
    printf("Hotel Name: %s\n", hotel_name);
    printf("Address: %s\n", address);
    printf("Phone: %s\n", phone);
    printf("Email: %s\n", email);
    printf("Facilities: %s\n", facilities);

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[10240];  // Twice the size of the largest array
    snprintf(query, sizeof(query),
             "UPDATE hotels SET name='%s', address='%s', phone='%s', email='%s', facilities='%s' "
             "WHERE name='%s'",
             hotel_name, address, phone, email, facilities, hotel_name);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        printf("Hotel data updated in the database\n");
    }

    // Clean up
    mysql_close(conn);
}

MYSQL *connect_to_DB() {
    MYSQL *conn;

    if ((conn = mysql_init(NULL)) == NULL) {
        fprintf(stderr, "Could not init DB\n");
        return NULL;
    }

    if (mysql_real_connect(conn, "localhost", "root", "secret", "project", 0, NULL, 0) == NULL) {
        fprintf(stderr, "DB Connection Error\n");
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

void send_customer_info_to_server(long long customer_id, int client_fd) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    // Prepare SQL statement
    char query[1024];
    snprintf(query, sizeof(query), "SELECT * FROM customers WHERE customerID = %lld", customer_id);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {
            // Check if the result set is not empty
            if (mysql_num_rows(result) > 0) {
                MYSQL_ROW row = mysql_fetch_row(result);

                // Fetch customer data
                // Note: Adjust the column indices based on your database schema
                long long id = atoll(row[0]);
                const char *firstname = row[1];
                const char *lastname = row[2];
                const char *address = row[3];
                const char *passport_number = row[4];
                const char *email = row[5];
                const char *phone_number = row[6];
                const char *username = row[7];
                const char *password = row[8];

                // Format the message with customer information
                char customer_info[1024];
                snprintf(customer_info, sizeof(customer_info),
                         "CUSTOMER_INFO|%lld|%s|%s|%s|%s|%s|%s|%s|%s",
                         id, firstname, lastname, address, passport_number,
                         email, phone_number, username, password);

                // Send the customer information to the client
                send(client_fd, customer_info, strlen(customer_info), 0);
            }

            mysql_free_result(result);
        }
    }

    // Clean up
    mysql_close(conn);
}
// nodira
void send_manager_info_to_server(long long manager_id, int client_fd) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    // Prepare SQL statement
    char query[1024];
    snprintf(query, sizeof(query), "SELECT * FROM managers WHERE managerID = %lld", manager_id);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {
            // Check if the result set is not empty
            if (mysql_num_rows(result) > 0) {
                MYSQL_ROW row = mysql_fetch_row(result);

                // Fetch customer data
                // Note: Adjust the column indices based on your database schema
                long long id = atoll(row[0]);
                const char *manager_firstname = row[1];
                const char *manager_lastname = row[2];
                const char *manager_address = row[3];
                const char *manager_passport_number = row[4];
                const char *manager_email = row[5];
                const char *manager_phone_number = row[6];
                const char *manager_username = row[7];
                const char *manager_password = row[8];

                // Format the message with manager information
                char manager_info[1024];
                snprintf(manager_info, sizeof(manager_info),
                         "MANAGER_INFO|%lld|%s|%s|%s|%s|%s|%s|%s|%s",
                         id, manager_firstname, manager_lastname, manager_address, manager_passport_number,
                         manager_email, manager_phone_number, manager_username, manager_password);

                // Send the manager information to the client
                send(client_fd, manager_info, strlen(manager_info), 0);
            }

            mysql_free_result(result);
        }
    }

    // Clean up
    mysql_close(conn);
}
void send_admin_info_to_server(long long admin_id, int client_fd) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    // Prepare SQL statement
    char query[1024];
    snprintf(query, sizeof(query), "SELECT * FROM admins WHERE adminID = %lld", admin_id);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {
            // Check if the result set is not empty
            if (mysql_num_rows(result) > 0) {
                MYSQL_ROW row = mysql_fetch_row(result);

                // Fetch customer data
                // Note: Adjust the column indices based on your database schema
                long long id = atoll(row[0]);
                const char *admin_firstname = row[1];
                const char *admin_lastname = row[2];
                const char *admin_username = row[3];
                const char *admin_password = row[4];
                const char *admin_email = row[5];
                const char *admin_phone_number = row[6];

                // Format the message with admin information
                char admin_info[1024];
                snprintf(admin_info, sizeof(admin_info),
                         "ADMIN_INFO|%lld|%s|%s|%s|%s|%s|%s",
                         id, admin_firstname, admin_lastname,admin_username, admin_password,
                         admin_email, admin_phone_number);

                // Send the admin information to the client
                send(client_fd, admin_info, strlen(admin_info), 0);
            }

            mysql_free_result(result);
        }
    }
    // Clean up
    mysql_close(conn);
}
void handle_manager_confirmation(const char *data) {
/*     char from_text[255], to_text[255], room_type_str[255], bed_type_str[255];
    unsigned int adult_value, child_value, baby_value;
    float total_price;
    long long client_id;

    if (sscanf(data, "FOR_MANAGER_CONFIRMATION|%12[^|]|%12[^|]|%u|%u|%u|%f|%19[^|]|%19[^|]|%lld",
                from_text, to_text, &adult_value, &child_value, &baby_value, &total_price,
                room_type_str, bed_type_str, &client_id) != 9) {
        fprintf(stderr, "Invalid FOR_MANAGER_CONFIRMATION data format: %s\n", data);
        return;
    }

    // Perform actions based on the received data
    // For example, you can print the received information
    printf("Received confirmation from client:\n");
    printf("From: %s\n", from_text);
    printf("To: %s\n", to_text);
    printf("Adults: %u, Children: %u, Babies: %u\n", adult_value, child_value, baby_value);
    printf("Total Price: %.2f\n", total_price);
    printf("Room Type: %s\n", room_type_str);
    printf("Bed Type: %s\n", bed_type_str);
    printf("--------------------------------------\n");

    // Send the information to the manager client */
    if (manager_fd != 0) {
        send(manager_fd, data, strlen(data), 0);
        printf("Sent confirmation to manager client\n");
    }
}

void *handle_connection(void *arg) {
    int client_fd = *((int *)arg);
    char buffer[1024];

    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    getpeername(client_fd, (struct sockaddr *)&client_addr, &addr_size);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);


    while (1) {
        memset(buffer, '\0', sizeof(buffer));
        ssize_t received_bytes = recv(client_fd, buffer, sizeof(buffer), 0);

        if (received_bytes <= 0) {
            if (received_bytes == 0) {
                // Client disconnected
                printf("[DISCONNECTED] Connection closed by client [%s]\n",client_ip);
            } else {
                perror("Error receiving data from the client");
            }
            break; // Exit the loop when the client disconnects
        }

        printf("[CLIENT] %s\n", buffer);

        if (strstr(buffer, "CLIENT_REGISTER|") != NULL) {
            handle_client_registration(buffer);
        }
        if (strstr(buffer, "MANAGER_REGISTER|") != NULL) {
            manager_fd=client_fd;
            handle_manager_registration(buffer);
        }
        if (strstr(buffer, "CLIENT_LOGIN|") != NULL) {
            handle_client_login(buffer, client_fd);
        }
        if (strstr(buffer, "MANAGER_LOGIN|") != NULL) {
            manager_fd=client_fd;
            handle_manager_login(buffer, client_fd);
        }
        if (strstr(buffer, "ADMIN_LOGIN|") != NULL) {
            handle_admin_login(buffer, client_fd);
        }
        if (strstr(buffer, "HOTELS|") != NULL) {
            const char *region_name = strchr(buffer, '|');
            if (region_name != NULL) {
                region_name++;
                retrieve_hotels(client_fd, region_name);
            }
        }
        if (strstr(buffer, "ADD_HOTEL|") != NULL) {
            handle_add_hotel(buffer);
        }
        if (strstr(buffer, "UPDATE_HOTEL|") != NULL) {
            handle_hotel_update(buffer);
        }
        if (strstr(buffer, "ALL_HOTELS") != NULL) {
            retrieve_all_hotels(client_fd);
        }
        if (strstr(buffer, "ALL_USERS") != NULL) {
            retrieve_all_customers(client_fd);
        }
        if (strstr(buffer, "ALL_MANAGERS") != NULL) {
            retrieve_all_managers(client_fd);
        }
        if (strstr(buffer, "HOTEL_NAME|") != NULL) {
            const char *hotelname = strchr(buffer, '|');
            if (hotelname != NULL) {
                hotelname++;
                retrieve_one_hotel(client_fd, hotelname);
            }
        }
        if (strstr(buffer, "CLIENT_UPDATE|") != NULL)
        {
            handle_client_update(buffer);
        }
        if (strstr(buffer, "MANAGER_UPDATE|") != NULL)
        {
            handle_manager_update(buffer);
        }
        if (strstr(buffer, "ADMIN_UPDATE|") != NULL)
        {
            handle_admin_update(buffer);
        }
        if (strstr(buffer, "CLIENT_INFO|") != NULL) {
             long long customer_id;
            if (sscanf(buffer, "CLIENT_INFO|%lld", &customer_id) == 1) {
                // Call the function to send customer information
                send_customer_info_to_server(customer_id, client_fd);
            } else {
                fprintf(stderr, "Invalid CLIENT_INFO format: %s\n", buffer);
            }
        }
        if (strstr(buffer, "MANAGER_INFO|") != NULL) {
             long long manager_id;
            if (sscanf(buffer, "MANAGER_INFO|%lld", &manager_id) == 1) {
                // Call the function to send customer information
                send_manager_info_to_server(manager_id, client_fd);
            } else {
                fprintf(stderr, "Invalid ADMIN_INFO format: %s\n", buffer);
            }
        }
        if (strstr(buffer, "ADMIN_INFO|") != NULL) {
             long long admin_id;
            if (sscanf(buffer, "ADMIN_INFO|%lld", &admin_id) == 1) {
                // Call the function to send customer information
                send_admin_info_to_server(admin_id, client_fd);
            } else {
                fprintf(stderr, "Invalid ADMIN_INFO format: %s\n", buffer);
            }
        }
        if (strstr(buffer, "FOR_MANAGER_CONFIRMATION|") != NULL)
        {
            handle_manager_confirmation(buffer);
        }
        
        
    }

    close(client_fd);
    //printf("[DISCONNECTED] Connection closed [%s]\n",client_ip);

    return NULL;
}

void handle_client_login(const char *data, int client_fd) {
    char username[100], password[100];
    if (sscanf(data, "CLIENT_LOGIN|%99[^|]|%99[^|]", username, password) != 2) {
        printf("LOGIN_DATA: [%s][%s]\n", username, password);
        fprintf(stderr, "Invalid login data format: %s\n", data);
        return;
    }

    // Print the received username and password for debugging
    printf("Received login request with username: %s, password: %s\n", username, password);

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    // Prepare SQL statement
    char query[1024];
    snprintf(query, sizeof(query), "SELECT customerID FROM customers WHERE username='%s' AND password='%s'", username, password);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);

        if (num_rows > 0) {
            MYSQL_ROW row = mysql_fetch_row(result);
            long long client_id = atoll(row[0]); // Assuming customerID is stored as a long long

            char response[256];
            snprintf(response, sizeof(response), "true|%lld", client_id);
            send(client_fd, response, strlen(response), 0);
            printf("[LOGIN_SUCCESS] Sent 'true' response with client ID (%lld) to client\n", client_id);
        } else {
            char response[] = "false";
            send(client_fd, response, strlen(response), 0);
            printf("[LOGIN_FAILURE] Sent 'false' response to client\n");
        }

        mysql_free_result(result);
    }

    // Clean up
    mysql_close(conn);
}

void handle_manager_login(const char *data, int client_fd) {
    char username[100], password[100];
    if (sscanf(data, "MANAGER_LOGIN|%99[^|]|%99[^|]", username, password) != 2) {
        printf("LOGIN_DATA FROM MANAGER: [%s][%s]\n", username, password);
        fprintf(stderr, "Invalid login data format: %s\n", data);
        return;
    }

    // Print the received username and password for debugging
    printf("Received login request with username: %s, password: %s\n", username, password);

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    // Prepare SQL statement
    char query[1024];
    snprintf(query, sizeof(query), "SELECT managerID FROM managers WHERE username='%s' AND password='%s'", username, password);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);

        if (num_rows > 0) {
            MYSQL_ROW row = mysql_fetch_row(result);
            long long manager_id = atoll(row[0]); // Assuming customerID is stored as a long long

            char response[256];
            snprintf(response, sizeof(response), "true|%lld", manager_id);
            send(client_fd, response, strlen(response), 0);
            printf("[LOGIN_SUCCESS] Sent 'true' response with client ID (%lld) to client\n", manager_id);
        } else {
            char response[] = "false";
            send(client_fd, response, strlen(response), 0);
            printf("[LOGIN_FAILURE] Sent 'false' response to client\n");
        }

        mysql_free_result(result);
    }

    // Clean up
    mysql_close(conn);
}

void handle_admin_login(const char *data, int client_fd) {
    char username[100], password[100];
    if (sscanf(data, "ADMIN_LOGIN|%99[^|]|%99[^|]", username, password) != 2) {
        printf("LOGIN_DATA: [%s][%s]\n", username, password);
        fprintf(stderr, "Invalid login data format: %s\n", data);
        return;
    }

    // Print the received username and password for debugging
    printf("Received login request with username: %s, password: %s\n", username, password);

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    // Prepare SQL statement
    char query[1024];
    snprintf(query, sizeof(query), "SELECT adminID FROM admins WHERE username='%s' AND password='%s'", username, password);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);

        if (num_rows > 0) {
            MYSQL_ROW row = mysql_fetch_row(result);
            long long admin_id = atoll(row[0]); // Assuming customerID is stored as a long long

            char response[256];
            snprintf(response, sizeof(response), "true|%lld", admin_id);
            send(client_fd, response, strlen(response), 0);
            printf("[LOGIN_SUCCESS] Sent 'true' response with client ID (%lld) to client\n", admin_id);
        } else {
            char response[] = "false";
            send(client_fd, response, strlen(response), 0);
            printf("[LOGIN_FAILURE] Sent 'false' response to client\n");
        }

        mysql_free_result(result);
    }

    // Clean up
    mysql_close(conn);
}

void handle_client_registration(const char *data) {
    long long customer_id;
    char firstname[255], lastname[255], address[255], passport_number[255], email[255], phone_number[15],
        username[255], password[255];

    if (sscanf(data, "CLIENT_REGISTER|%lld|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
               &customer_id, firstname, lastname, address, passport_number, email, phone_number,
               username, password) != 9) {
        fprintf(stderr, "Invalid registration data format: %s\n", data);
        return;
    }

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[2048];
    snprintf(query, sizeof(query),
             "INSERT INTO customers (customerID, firstname, lastname, address, passport_number, email, phone_number, username, password) "
             "VALUES (%lld, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
             customer_id, firstname, lastname, address, passport_number, email, phone_number, username, password);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        printf("Customer data inserted into the database\n");
    }

    // Clean up
    mysql_close(conn);
}

void handle_client_update(const char *data) {
    long long customer_id;
    char firstname[255], lastname[255], address[255], passport_number[255], email[255], phone_number[15],
        username[255], password[255];

    if (sscanf(data, "CLIENT_UPDATE|%lld|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
               &customer_id, firstname, lastname, address, passport_number, email, phone_number,
               username, password) != 9) {
        fprintf(stderr, "Invalid update data format: %s\n", data);
        return;
    }
    printf("UPDATING!!! [%s]",data);

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[2048];
    snprintf(query, sizeof(query),
             "UPDATE customers SET firstname='%s', lastname='%s', address='%s', passport_number='%s', email='%s', phone_number='%s', username='%s', password='%s' "
             "WHERE customerID=%lld",
             firstname, lastname, address, passport_number, email, phone_number, username, password, customer_id);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        printf("Customer data updated in the database\n");
    }

    // Clean up
    mysql_close(conn);
}

void handle_manager_update(const char *data) {
    long long manager_id;
    char manager_firstname[255], manager_lastname[255], manager_address[255], manager_passport_number[255], manager_email[255], manager_phone_number[15],
        manager_username[255], manager_password[255];

    if (sscanf(data, "MANAGER_UPDATE|%lld|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
               &manager_id, manager_firstname, manager_lastname, manager_address, manager_passport_number, manager_email, manager_phone_number,
               manager_username, manager_password) != 9) {
        fprintf(stderr, "Invalid update data format: %s\n", data);
        return;
    }
    printf("UPDATING!!! [%s]",data);

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[2048];
    snprintf(query, sizeof(query),
             "UPDATE managers SET firstname='%s', lastname='%s', address='%s', passport_number='%s', email='%s', phone_number='%s', username='%s', password='%s' "
             "WHERE managerID=%lld",
             manager_firstname, manager_lastname, manager_address, manager_passport_number, manager_email, manager_phone_number, manager_username, manager_password, manager_id);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        printf("Manager data updated in the database\n");
    }

    // Clean up
    mysql_close(conn);
}

void handle_admin_update(const char *data) {
    long long admin_id;
    char admin_firstname[255], admin_lastname[255], admin_address[255], admin_passport_number[255], admin_email[255], admin_phone_number[15],
        admin_username[255], admin_password[255];

    if (sscanf(data, "ADMIN_UPDATE|%lld|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
               &admin_id, admin_firstname, admin_lastname, admin_address, admin_passport_number, admin_email, admin_phone_number,
               admin_username, admin_password) != 9) {
        fprintf(stderr, "Invalid update data format: %s\n", data);
        return;
    }
    printf("UPDATING!!! [%s]",data);

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[2048];
    snprintf(query, sizeof(query),
             "UPDATE admins SET firstname='%s', lastname='%s', address='%s', passport_number='%s', email='%s', phone_number='%s', username='%s', password='%s' "
             "WHERE customerID=%lld",
             admin_firstname, admin_lastname, admin_address, admin_passport_number, admin_email, admin_phone_number, admin_username, admin_password, admin_id);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        printf("Admin data updated in the database\n");
    }

    // Clean up
    mysql_close(conn);
}

void handle_add_hotel(const char *data) {
    char hotel_name[255], address[255], email[255], phone[15], rating[255], region[255], image[1000], facilities[255];

    if (sscanf(data, "ADD_HOTEL|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
               hotel_name, address, email, phone, rating, region, image, facilities) != 8) {
        fprintf(stderr, "Invalid ADD_HOTEL data format: %s\n", data);
        return;
    }

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[4096];
    snprintf(query, sizeof(query),
             "INSERT INTO hotels (name, address, email, phone, rating, region, picture, facilities) "
             "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
             hotel_name, address, email, phone, rating, region, image, facilities);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        printf("Hotel data inserted into the database\n");
    }

    // Clean up
    mysql_close(conn);
}

void handle_manager_registration(const char *data) {
    long long manager_id;
    char firstname[255], lastname[255], address[255], passport_number[255], email[255], phone_number[15],
        username[255], password[255],hotel_name[255];

    if (sscanf(data, "MANAGER_REGISTER|%lld|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
               &manager_id, firstname, lastname, address, passport_number, email, phone_number,
               username, password,hotel_name) != 10) {
        fprintf(stderr, "Invalid MANAGER registration data format: %s\n", data);
        return;
    }

    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    char query[4096];
    snprintf(query, sizeof(query),
             "INSERT INTO managers (managerID, firstname, lastname, address, passport_number, email, phone_number, username, password, hotel_name) "
             "VALUES (%lld, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
             manager_id, firstname, lastname, address, passport_number, email, phone_number, username, password,hotel_name);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        printf("MANAGER data inserted into the database\n");
    }

    // Clean up
    mysql_close(conn);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];

    // Server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    //server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_addr.s_addr = inet_addr("172.18.0.29");
//    server_addr.sin_addr.s_addr = inet_addr("192.168.245.242");
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("[LISTENING] Port Number: %d\n", PORT);

while (1) {
    addr_size = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

    if (client_fd == -1) {
        perror("Accept failed");
        continue;
    }
        // Convert the client's IP address from binary to a readable string
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

    printf("[CONNECTED] New connection from %s\n", client_ip);

    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, handle_connection, &client_fd) != 0) {
        perror("Failed to create thread");
        close(client_fd);
        continue;
    }

    pthread_detach(client_thread);
}

    close(server_fd);

    return 0;
}


