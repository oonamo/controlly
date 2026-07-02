clc; clear;
% 2-States, 2-Inputs, 2-Outputs
A = [-1,  1; 
     -1, -2];
     
B = [1, 2; 
     3, 4];
     
C = [5, 6; 
     7, 8];
     
D = [1, 0; 
     0, 1];

Ts = 0.1;       % 100ms sample time
x = [0; 0];     % Initial states
u = [1; 1];     % Constant step command on both inputs

% 3 Euler iterations
for k = 1:3
    y = C*x + D*u;
    x_dot = A*x + B*u;
    x = x + (x_dot * Ts);
end

fprintf('Expected Final X: {%.3ff, %.3ff}\n', x(1), x(2));
fprintf('Expected Final Y: {%.3ff, %.3ff}\n', y(1), y(2));