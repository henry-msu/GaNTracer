% the file to read results from
file = "caltest48.csv";

data = readtable(file);

fprintf("\nReliability data from %d temperature readings\n", length(data.calTempC))
fprintf("Maximum temperature recorded: %.02f °C\n", max(data.calTempC))
fprintf("Minimum temperature recorded: %.02f °C\n", min(data.calTempC))

fprintf("Largest difference in temperature readings: %.02f °C\n", max(data.calTempC) - min(data.calTempC))

n = 1:length(data.rawData);

maxC = max(data.calTempC);
minC = min(data.calTempC);
maxF = max(data.calTempF);
minF = min(data.calTempF);
figure(1);
clf
plot(n, data.calTempC, 'b');
set(gca,"FontSize",20);
title("temperature measurements");
ylabel("Temperature ({\circ}C)", "FontSize", 20);
xlabel("Measurement number");
xlim([0 max(n)])
ylim([minC-0.5 maxC+0.5]);
% figure(2);
% plot(n, data.calTempF); title("temperature measurements"); ylabel("Temperature ({\circ}F)");
% ylim([minF-2 maxF+2]);