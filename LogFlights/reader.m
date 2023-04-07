accx = 1;
accy = 2;
accz = 3;
gyrox = 4;
gyroy = 5;
gyroz = 6;
magnx = 7;
magny = 8;
magnz = 9;
roll = 10;
pitch = 11;
yaw = 12;
roll_var = 13;
pitch_var = 14;
yaw_var = 15;
kf_dt = 16;
throttle = 17;
cmd_roll = 18;
cmd_pitch = 19;
cmd_yaw = 20;
pid_roll = 21;
pid_pitch = 22;
pid_yaw = 23;
motors_armed = 24;
m1_speed = 25;
m2_speed = 26;
m3_speed = 27;
m4_speed = 28;

M = csvread("28-03-2023-02-49.csv", 2, 0);

hold all;
plot(M(:, roll),  'DisplayName', 'ROLL');
plot(M(:, pitch), 'DisplayName', 'PITCH');
plot(M(:, m1_speed) - 1100,  'DisplayName', 'M1 SPEED');
%plot(M(:, m2_speed) - 1100,  'DisplayName', 'M2 SPEED');
%plot(M(:, m3_speed) - 1100,  'DisplayName', 'M3 SPEED');
%plot(M(:, m4_speed),  'DisplayName', 'M4 SPEED');
legend(gca, 'show');

grid on;
