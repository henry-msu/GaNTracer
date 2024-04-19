% the file to read results from
file = "accuracytest1.csv";

% successful test criteria in degrees C
maxErrorHigh = 1;
maxErrorLow = -1;

data = readtable(file);
n = 1:length(data.rawData);
maxC = max(data.calTempC);
minC = min(data.calTempC);
maxF = max(data.calTempF);
minF = min(data.calTempF);
actualTempF = round(data.actualTempC * (9/5) + 32, 2);

figure(1);
clf
plot(n, data.calTempC, 'b', 'LineWidth', 3); title("Calibrated temperature measurements", "FontSize", 14)
 ylabel("Temperature ({\circ}C)", "FontSize", 16); set(gca,"FontSize",20);
hold on;
plot(n, data.actualTempC, 'r', 'LineWidth', 3);
legend("Measured temperature", "Actual temperature", "FontSize", 22, "Location", 'northwest');
xlim([min(n), max(n)]);

figure(2);
clf
hold on;
yline(maxError, 'r--', ['maximum positive error to meet spec: ', num2str(maxErrorHigh), ' {\circ}C'], 'LineWidth', 2);
yline(minError, 'r--', ['maximum positive error to meet spec: ', num2str(maxErrorLow), ' {\circ}C'], 'LineWidth', 2);
plot(n, data.tempErrorC, 'b',  'LineWidth', 3);
title("Error in temperature", "FontSize", 14);
ylabel("Temperature error ({\circ}C)", "FontSize", 16)
xlabel("Temperature reading", "FontSize", 16);

set(gca,"FontSize",20);

ylim padded;
xlim([min(n), max(n)]);

fprintf('\n-------------------------------------------------\n');
fprintf('Accuracy test results for "%s"\n', file);
fprintf('-------------------------------------------------\n');
if (max(data.tempErrorC) <= maxErrorHigh && min(data.tempErrorC) >= maxErrorLow)
    fprintf('Test from file "%s" passed accuracy testing\n', file);
else
    fprintf('Test from file "%s" FAILED accuracy testing\n', file);
end
fprintf('Temperature readings: %d\n', length(n));
fprintf('Maximum positive error allowed: %.02f °C\n', maxErrorHigh);
fprintf('Maximum negative error allowed: %.02f °C\n', maxErrorLow);
fprintf('Maximum temperature error: %.03f °C\n', max(data.tempErrorC));
fprintf('Minimum temperature error: %.03f °C\n', min(data.tempErrorC));
fprintf('See figures for more details\n');