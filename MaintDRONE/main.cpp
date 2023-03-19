#include "MAINT.h"

#include <iostream>
#include <Windows.h>

int main(int argc, char** argv)
{
    HANDLE hComm = CreateFileA(argv[1], //PORT NAME
        GENERIC_READ | GENERIC_WRITE,   //READ/WRITE
        0,                              //NO SHARING
        NULL,                           //NO SECURITY
        OPEN_EXISTING,                  //OPEN EXISTING PORT ONLY
        0,                              //NON OVERLAPPED I/O
        NULL);                          //NULL FOR COMM DEVICES


    if (hComm == INVALID_HANDLE_VALUE) {
        return 2;
    }

    DCB commStateConfig;

    if (!GetCommState(hComm, &commStateConfig)) {
        return 3;
    }

    commStateConfig.BaudRate = 115200;
    commStateConfig.ByteSize = 8;

    if (!SetCommState(hComm, &commStateConfig)) {
        return 4;
    }

    FILE* f = fopen("log.py", "w");
    fwrite("values = [ \\\n", 1, strlen("values = [ \\\n"), f);
    fclose(f);
    int id = 0;
    while (1)
    {
        DWORD size;
        BOOL res = ReadFile(hComm, &packet, 4, &size, 0);
        if (res)
        {
            if (packet.sync == SYNC_SEQ)
            {
                std::cout << "FOUND SYNC" << std::endl;
                res = ReadFile(hComm, &packet.sw_ver, sizeof(maint_data_t) - 4, &size, 0);
                if (res)
                {
                    //std::cout << "SW-VER: " << packet.sw_ver[0] << "." << packet.sw_ver[1] << "." << packet.sw_ver[2] << std::endl;
                    //std::cout << "GYRO_X: " << packet.gyro[0] << " GYRO_Y: " << packet.gyro[1] << " GYRO_Z: " << packet.gyro[2] << std::endl;
                    //std::cout << "ACC_X: "  << packet.acc[0] << " ACC_Y: " << packet.acc[1] << " ACC_Z: " << packet.acc[2] << std::endl;
                    //std::cout << "MAGN_X: " << packet.magn[0] << " MAGN_Y: " << packet.magn[1] << " MAGN_Z: " << packet.magn[2] << std::endl;
                    //std::cout << "IMU_T: "  << packet.t << std::endl;
                    //std::cout << "KF_DT: "  << packet.dt << std::endl;
                    //std::cout << "ROLL: "   << packet.attitude[0] << " PITCH: " << packet.attitude[1] << " YAW: " << packet.attitude[2] << std::endl;
                    //std::cout << "R-CHAN-1: " << packet.radio_channels[0] << std::endl
                      //        << "R-CHAN-2: " << packet.radio_channels[1] << std::endl
                        //      << "R-CHAN-3: " << packet.radio_channels[2] << std::endl
                          //    << "R-CHAN-4: " << packet.radio_channels[3] << std::endl
                            //  << "R-CHAN-5: " << packet.radio_channels[4] << std::endl;
                    //std::cout << "M-ARMED: " << packet.motors_armed << std::endl;
                    //std::cout << "M1-SPEED: " << packet.motors_speed[0] << std::endl;
                    //std::cout << "M2-SPEED: " << packet.motors_speed[1] << std::endl;
                    //std::cout << "M3-SPEED: " << packet.motors_speed[2] << std::endl;
                    //std::cout << "M4-SPEED: " << packet.motors_speed[3] << std::endl;
                    FILE* f = fopen("log.py", "a");
                    fprintf(f, "%d, %.6f, %.6f, %.6f;\\\n", id++, packet.attitude[0], packet.attitude[1], packet.attitude[2]);
                    fclose(f);

                }
            }

        }
    }

	return 0;
}