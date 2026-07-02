A = [0 1; -5 -2];
B = [0; 1];
C = [10 0];
D = 0;

Ts = 0.1;    % 100ms sample time for the simulation
x = [0; 0];  % Initial states at zero
u = 1;       % Step input of 1.0

% 3 Euler iterations
for k = 1:3
    y = C*x + D*u;
    x_dot = A*x + B*u;
    x = x + (x_dot * Ts);
end

fprintf('Expected Final X: {%.3ff, %.3ff}\n', x(1), x(2));
fprintf('Expected Final Y: {%.3ff}\n', y(1));
