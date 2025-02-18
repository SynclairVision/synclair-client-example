#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <sstream>
#include <string>
#include <mutex>
#include <unistd.h>

#ifdef _WIN32
#include <Ws2tcpip.h>
#include <winsock.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include "message-definitions/msg_defs.hpp"

// Implementering av de nya funktionerna
void pack_get_battery_status(message &msg) {
    msg.message_type = BATTERY_STATUS;
    // Lägg till ytterligare logik om nödvändigt
}

void pack_start_video_recording(message &msg) {
    msg.message_type = START_VIDEO_RECORDING;
    // Lägg till ytterligare logik om nödvändigt
}

void pack_stop_video_recording(message &msg) {
    msg.message_type = STOP_VIDEO_RECORDING;
    // Lägg till ytterligare logik om nödvändigt
}

void pack_take_photo(message &msg) {
    msg.message_type = TAKE_PHOTO;
    // Lägg till ytterligare logik om nödvändigt
}

static constexpr unsigned int DEFAULT_PORT = 8555;
static constexpr message EMPTY_MESSAGE = {0, VERSION, EMPTY, 0, {0}};
static constexpr message QUIT_MESSAGE  = {0, VERSION, QUIT,  0, {0}};

int server_socket = -1;

std::vector<message> messages_to_send;
std::mutex mtx;

bool read_messages = true;
bool write_messages = true;

// Tries to connect to server at port 8555
bool try_connect(std::string ip) {
    // Create a socket
    std::cout<<"Connecting to "<<ip<<"...\n";
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Connect to the server
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);


    if(connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        std::cout<<"Could not connect to "<<ip<<"\n";
        return false;
    }

    std::cout<<"Connected to "<<ip<<"\n";
    return true;
}

void close_connection() {
    if(server_socket != -1) {
#ifdef _WIN32
        closesocket(server_socket);
        WSACleanup();
#else
        close(server_socket);
#endif
        server_socket = -1;
    }
}

void send_message(const message &msg) {
    if(server_socket == -1) {
        std::cout<<"Not connected to server\n";
        return;
    }

    char* ser_msg = serialize_message(msg);
    size_t bytes_sent = send(server_socket, ser_msg, sizeof(msg), 0);

    if (bytes_sent == -1) {
#ifdef _WIN32
        std::cout << "Error during send: " << WSAGetLastError() << "\n";
#else
        std::cout << "Error during send: " << strerror(errno) << "\n";
#endif
    }

    delete[] ser_msg;
}

message read_message() {
    if(server_socket == -1) {
        std::cout<<"Not connected to server\n";
        return EMPTY_MESSAGE;
    }

    char buffer[sizeof(message)] = {0};
    int bytes_read = recv(server_socket, buffer, sizeof(buffer), 0);

    // Data received successfully
    if (bytes_read > 0) {
        return deserialize_message(buffer);
    }
    // Connection closed by peer
    else if (bytes_read == 0) {
        std::cerr << "SV connection closed by peer." << "\n";
        return QUIT_MESSAGE;
    }
    // Handle other error cases
    else {
#ifdef _WIN32
        std::cerr << "Error during recv: " << WSAGetLastError() << "\n";
#else
        std::cerr << "Error during recv: " << strerror(errno) << "\n";
#endif
        return QUIT_MESSAGE;
    }
}

void handle_current_parameters(message &msg) {
    printf("--------------------\n");
    switch(msg.param_type) {
        case SYSTEM_STATUS:
        {
            system_status_parameters params;
            unpack_system_status_parameters(msg, params);
            printf(
                "System status\n\
                \tStatus: %d\n\
                \tError: %d\n",
                params.status,
                params.error
            );
            break;
        }
        case GENERAL_SETTINGS:
        {
            general_settings_parameters params;
            unpack_general_settings_parameters(msg, params);
            printf(
                "General settings\n\
                \tCamera width: %u\n\
                \tCamera height: %u\n\
                \tROI width: %u\n\
                \tROI height: %u\n\
                \tCamera FPS: %u\n\
                \tMount yaw: %f\n\
                \tMount pitch: %f\n\
                \tMount roll: %f\n\
                \tRun AI: %u\n",
                params.camera_width,
                params.camera_height,
                params.roi_width,
                params.roi_height,
                (uint16_t)params.camera_fps,
                params.mount_yaw,
                params.mount_pitch,
                params.mount_roll,
                (uint16_t)params.run_ai
            );
            break;
        }
        case VIDEO_OUTPUT:
        {
            video_output_parameters params;
            unpack_video_output_parameters(msg, params);
            uint16_t layout_mode = params.layout_mode & 0x0f;
            uint16_t num_views = (params.layout_mode & 0xf0) >> 4;
            printf(
                "Video output\n\
                \tWidth: %u\n\
                \tHeight: %u\n\
                \tFPS: %u\n\
                \tLayout mode: %u\n\
                \tNumber of views: %u\n\
                \tDetection overlay mode: %u\n\
                \tDetection overlay box: X: %u, Y: %u, W: %u, H: %u\n\
                \tSingle detection size: %u\n",
                params.width,
                params.height,
                (uint16_t)params.fps,
                (uint16_t)layout_mode,
                (uint16_t)num_views,
                (uint16_t)params.detection_overlay_mode,
                params.detection_overlay_box.x,
                params.detection_overlay_box.y,
                params.detection_overlay_box.w,
                params.detection_overlay_box.h,
                params.single_detection_size
            );
            for(uint16_t i = 0; i < num_views; i++) {
                printf(
                    "View %u\n\
                    \tX: %u\n\
                    \tY: %u\n\
                    \tW: %u\n\
                    \tH: %u\n",
                    i,
                    params.views[i].x,
                    params.views[i].y,
                    params.views[i].w,
                    params.views[i].h
                );
            }
            break;
        }
        case CAPTURE:
        {
            capture_parameters params;
            unpack_capture_parameters(msg, params);
            printf(
                "Capture\n\
                \tCapture single image: %u\n\
                \tRecord video: %u\n\
                \tImages captured: %u\n\
                \tVideos captured: %u\n",
                (uint16_t)params.cap_single_image,
                (uint16_t)params.record_video,
                params.images_captured,
                params.videos_captured
            );
            break;
        }
        case DETECTION:
        {
            detection_parameters params;
            unpack_detection_parameters(msg, params);
            printf(
                "Detection settings\n\
                \tMode: %u\n\
                \tSorting mode: %u\n\
                \tCrop confidence threshold: %f\n\
                \tVar confidence threshold: %f\n\
                \tCrop box limit: %u\n\
                \tVar box limit: %u\n\
                \tCrop box overlap: %f\n\
                \tVar box overlap: %f\n\
                \tCreation score scale: %u\n\
                \tBonus detection scale: %u\n\
                \tBonus redetection scale: %u\n\
                \tMissed detection penalty: %u\n\
                \tMissed redetection penalty: %u\n",
                (uint16_t)params.mode,
                (uint16_t)params.sorting_mode,
                params.crop_confidence_threshold,
                params.var_confidence_threshold,
                params.crop_box_limit,
                params.var_box_limit,
                params.crop_box_overlap,
                params.var_box_overlap,
                (uint16_t)params.creation_score_scale,
                (uint16_t)params.bonus_detection_scale,
                (uint16_t)params.bonus_redetection_scale,
                (uint16_t)params.missed_detection_penalty,
                (uint16_t)params.missed_redetection_penalty
            );
            break;
        }
        case DETECTED_ROI:
        {
            detected_roi_parameters params;
            unpack_detected_roi_parameters(msg, params);
            printf(
                "Detected ROI\n\
                \tIndex: %u\n\
                \tScore: %u\n\
                \tTotal detections: %u\n\
                \tYaw abs: %f\n\
                \tPitch abs: %f\n\
                \tYaw rel: %f\n\
                \tPitch rel: %f\n\
                \tLatitude: %f\n\
                \tLongitude: %f\n\
                \tAltitude: %f\n\
                \tDistance: %f\n",
                (uint16_t)params.index,
                (uint16_t)params.score,
                (uint16_t)params.total_detections,
                params.yaw_abs,
                params.pitch_abs,
                params.yaw_rel,
                params.pitch_rel,
                params.latitude,
                params.longitude,
                params.altitude,
                params.distance
            );
            break;
        }
        case CAM_EULER:
        {
            cam_euler_parameters params;
            unpack_cam_euler_parameters(msg, params);
            printf(
                "Camera Euler\n\
                \tCamera ID: %u\n\
                \tIs delta: %u\n\
                \tYaw: %f\n\
                \tPitch: %f\n\
                \tRoll: %f\n",
                (uint16_t)params.cam_id,
                (uint16_t)params.is_delta,
                params.yaw,
                params.pitch,
                params.roll
            );
            break;
        }
        case CAM_ZOOM:
        {
            cam_zoom_parameters params;
            unpack_cam_zoom_parameters(msg, params);
            printf(
                "Camera zoom\n\
                \tCamera ID: %u\n\
                \tZoom: %d\n",
                (uint16_t)params.cam_id,
                (int16_t)params.zoom
            );
            break;
        }
        case CAM_LOCK_FLAGS:
        {
            cam_lock_flags_parameters params;
            unpack_cam_lock_flags_parameters(msg, params);
            printf(
                "Camera lock flags\n\
                \tCamera ID: %u\n\
                \tFlags: Yaw: %u, Pitch: %u, Roll: %u\n",
                (uint16_t)params.cam_id,
                (uint16_t)(params.flags & 0x04),
                (uint16_t)(params.flags & 0x02),
                (uint16_t)(params.flags & 0x01)
            );
            break;
        }
        case CAM_CONTROL_MODE:
        {
            cam_control_mode_parameters params;
            unpack_cam_control_mode_parameters(msg, params);
            printf(
                "Camera control mode\n\
                \tCamera ID: %u\n\
                \tMode: %u\n",
                (uint16_t)params.cam_id,
                (uint16_t)params.mode
            );
            break;
        }
        case CAM_CROP_MODE:
        {
            cam_crop_mode_parameters params;
            unpack_cam_crop_mode_parameters(msg, params);
            printf(
                "Camera crop mode\n\
                \tCamera ID: %u\n\
                \tMode: %u\n",
                (uint16_t)params.cam_id,
                (uint16_t)params.mode
            );
            break;
        }
        case CAM_OFFSET:
        {
            cam_offset_parameters params;
            unpack_cam_offset_parameters(msg, params);
            printf(
                "Camera offset\n\
                \tCamera ID: %u\n\
                \tX: %f\n\
                \tY: %f\n\
                \tAbsolute Yaw: %f\n\
                \tAbsolute Pitch: %f\n\
                \tRelative Yaw: %f\n\
                \tRelative Pitch: %f\n",
                (uint16_t)params.cam_id,
                params.x,
                params.y,
                params.yaw_abs,
                params.pitch_abs,
                params.yaw_rel,
                params.pitch_rel
            );
            break;
        }
        case CAM_FOV:
        {
            cam_fov_parameters params;
            unpack_cam_fov_parameters(msg, params);
            printf(
                "Camera FOV\n\
                \tCamera ID: %u\n\
                \tFOV: %f\n",
                (uint16_t)params.cam_id,
                params.fov
            );
            break;
        }
        case CAM_TARGET:
        {
            cam_target_parameters params;
            unpack_cam_target_parameters(msg, params);
            printf(
                "Camera target\n\
                \tCamera ID: %u\n\
                \tX: %f\n\
                \tY: %f\n\
                \tLatitude: %f\n\
                \tLongitude: %f\n\
                \tAltitude: %f\n",
                (uint16_t)params.cam_id,
                params.x,
                params.y,
                params.t_latitude,
                params.t_longitude,
                params.t_altitude
            );
            break;
        }
        case CAM_SENSOR:
        {
            cam_sensor_parameters params;
            unpack_cam_sensor_parameters(msg, params);
            printf(
                "Camera sensor\n\
                \tAE: %u\n\
                \tTarget brightness: %u\n\
                \tExposure value: %u\n\
                \tGain value: %u\n",
                (uint16_t)params.ae,
                (uint16_t)params.target_brightness,
                params.exposure_value,
                params.gain_value
            );
            break;
        }
       
        default:
            printf("Unknown message type\n");
            break;
    }
    printf("--------------------\n\n");
}

void read_loop() {
    int timeout_ms = 50;
    while(read_messages) {
        message msg = read_message();

        if(msg.message_type == QUIT) {
            write_messages = false;
            read_messages = false;
            close_connection();
            break;
        }
        if(msg.message_type == CURRENT_PARAMETERS) {
            handle_current_parameters(msg);
        }
        // Read in 20 hz
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    }
}

void write_loop() {
    int timeout_ms = 50;
    while(write_messages) {
        mtx.lock();
        if(messages_to_send.size() > 0) {
            for(message &msg : messages_to_send) {
                send_message(msg);
            }
            messages_to_send.clear();
        }
        mtx.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cout<<"Usage: "<<argv[0]<<" <ip>\n";
        return 1;
    }

#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

    std::string ip = argv[1];
    if(!try_connect(ip)) {
        return 1;
    }

    std::thread read_thread(read_loop);
    std::thread write_thread(write_loop);

    // Send GET for one of each message
    mtx.lock();
    message msg;
    pack_get_parameters(msg, SYSTEM_STATUS);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, GENERAL_SETTINGS);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, VIDEO_OUTPUT);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, CAPTURE);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, DETECTION);
    messages_to_send.push_back(msg);

    /* This message comes in three forms:
       1. Specify index
       2. Get all visible on screen
       3. Get all
       For this example we read all (option 3) */

    //pack_get_detected_roi(msg, 0);
    //pack_get_detected_roi_visible(msg);
    pack_get_detected_roi_all(msg);
    messages_to_send.push_back(msg);

    // For these messages we need to specify the camera index
    pack_get_parameters(msg, CAM_EULER, 0);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, CAM_ZOOM, 0);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, CAM_LOCK_FLAGS, 0);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, CAM_CONTROL_MODE, 0);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, CAM_CROP_MODE, 0);
    messages_to_send.push_back(msg);

    // Top left corner of the screen
    pack_get_cam_offset_parameters(msg, 0, -1.f, -1.f);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, CAM_FOV, 0);
    messages_to_send.push_back(msg);

    pack_get_parameters(msg, CAM_TARGET, 0);
    messages_to_send.push_back(msg);

    // Back to messages non camera specific
    pack_get_parameters(msg, CAM_SENSOR);
    messages_to_send.push_back(msg);

    mtx.unlock();

    read_thread.join();
    write_thread.join();
    return 0;
}
