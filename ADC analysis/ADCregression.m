function [regression, gain, offset] = ADCregression(independent, dependent)
% ADCREGRESSION Calculates a linear regression using two variables and
% returns the MATLAB linear regression, as well as the gain and offset
% coefficients
%   USAGE: [regression, gain, offset] = ADCregression(independent, dependent)
%       independent = independent variable
%         dependent = dependent variable
%        regression = MATLAB linear regression model
%              gain = gain coefficient (slope)
%            offset = offset coefficient (y-intercept)
regression = fitlm(independent, dependent);

coefficients = regression.Coefficients.Estimate;
offset = coefficients(1);
gain = coefficients(2);
end

