function [compensated_values, error, errorpercent] = compensate(measured, gain, offset)
%ADCCOMPENSATE Perform compensation of measured value with gain and offset
%compensation
compensated_values = measured*gain + offset;
end