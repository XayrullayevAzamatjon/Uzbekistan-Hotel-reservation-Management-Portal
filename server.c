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

#define PORT 4462
void *handle_connection(void *arg);
void handle_client_login(const char *data, int client_fd);
void handle_manager_login(const char *data, int client_fd);
void handle_client_registration(const char *data);
void handle_manager_registration(const char *data);
void send_customer_info_to_server(long long customer_id, int client_fd);
MYSQL *connect_to_DB() ;
void handle_client_update(const char *data) ;

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

void send_hotel_names(int client_fd) {
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    const char *query = "SELECT hotelID, name FROM hotels";
    
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);

        if (result != NULL) {
            int num_names = mysql_num_rows(result);

            // Send the number of hotel names
            send(client_fd, &num_names, sizeof(num_names), 0);

            // Send each hotel name with ID
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != NULL) {
                HotelName hotelName;
                hotelName.id = atoll(row[0]); // Convert string to long long
                snprintf(hotelName.name, sizeof(hotelName.name), "%s", row[1]); // Copy name

                send(client_fd, &hotelName, sizeof(HotelName), 0);
            }

            mysql_free_result(result);
        }
    }

    mysql_close(conn);
}


void retrieve_hotels(int client_fd) {
    // Connect to MySQL
    MYSQL *conn = connect_to_DB();

    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    const char *query = "SELECT * FROM hotels";

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
                snprintf(hotel.name, sizeof(hotel.name), "%s", row[1]);         // Copy name
                snprintf(hotel.address, sizeof(hotel.address), "%s", row[2]);   // Copy address
                snprintf(hotel.phone, sizeof(hotel.phone), "%s", row[3]);       // Copy phone
                snprintf(hotel.email, sizeof(hotel.email), "%s", row[4]);       // Copy email
                hotel.rating = strtof(row[5], NULL);                            // Convert and copy rating
                snprintf(hotel.facilities, sizeof(hotel.facilities), "%s", row[6]);  // Copy facilities
                snprintf(hotel.picture, sizeof(hotel.picture), "%s", row[7]);  // Copy picture

                send(client_fd, &hotel, sizeof(Hotel), 0);

            }


            mysql_free_result(result);
        }
    }

 
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
            handle_manager_registration(buffer);
        }
        if (strstr(buffer, "CLIENT_LOGIN|") != NULL) {
            handle_client_login(buffer, client_fd);
        }
        if (strstr(buffer, "MANAGER_LOGIN|") != NULL) {
            handle_manager_login(buffer, client_fd);
        }
        if (strstr(buffer, "HOTELS") != NULL)
        {
            retrieve_hotels(client_fd);
        }
        if (strstr(buffer, "NAMES| ") != NULL)
        {
            send_hotel_names(client_fd);
        }
        if (strstr(buffer, "CLIENT_UPDATE|") != NULL)
        {
            handle_client_update(buffer);
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
    snprintf(query, sizeof(query), "SELECT * FROM managers WHERE username='%s' AND password='%s'", username, password);

    // Execute SQL statement
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Failed to execute query: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);

        if (num_rows > 0) {
            char response[] = "true";
            send(client_fd, response, strlen(response), 0);
            printf("[LOGIN_SUCCESS] Sent 'true' response to client\n");
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
    server_addr.sin_addr.s_addr = INADDR_ANY;

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


