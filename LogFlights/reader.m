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

M = csvread("08-04-2023-03-31.csv", 2, 0);

pkg load signal;
dt = 0.020;
fs = 1/dt;
fc = 10;
fn =  fc/(fs/2)
[b,a] = butter(6,fn);

ax = M(:,accx);
ay = M(:,accy);
az = M(:,accz);
N_samples = length(ax);

accx_filtered = zeros(1, N_samples);
accy_filtered = zeros(1, N_samples);
accz_filtered = zeros(1, N_samples);
for i=1:N_samples
  accx_filtered(i) = filter(b,a,ax(i));
  accy_filtered(i) = filter(b,a,ay(i));
  accz_filtered(i) = filter(b,a,az(i));
endfor


hold all;
plot(accx_filtered, 'DisplayName', 'ACCX_Filtered');
plot(accy_filtered, 'DisplayName', 'ACCY_Filtered');
plot(accz_filtered, 'DisplayName', 'ACCZ_Filtered');
plot(M(:, accx),  'DisplayName', 'ACCX');
plot(M(:, accy),  'DisplayName', 'ACCY');
plot(M(:, accz),  'DisplayName', 'ACCZ');

legend(gca, 'show');

grid on;
