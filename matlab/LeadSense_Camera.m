clc;
disp('### EvoBinoSDK With Matlab (Images and Normalized Depth) ###');
disp('### Since Matlab mxArray has column major order, and evo::Mat has row major order, this sample application is not designed to run in real time ###');
close all;
clear mex;
clear functions;
clear all;

mexLeadSense('create');

result = mexLeadSense('open', 3) %RESOLUTION_FPS_MODE_HD800_60

if(strcmp(result, 'OK'))
    % Get Image Size & FPS
    image_size_fps = mexLeadSense('getImageSizeFPS')
    
    % Set Confidence Threshold (98%)
    mexLeadSense('setConfidenceThreshold', 0.02);
    
    % Define maximum distance z (unit: mm)
    maxDistanceZ = 8000;
    binranges = 0:100:maxDistanceZ;
    mexLeadSense('setDistanceMaxValue', maxDistanceZ);
    
    % Create Figure and wait for keyboard interruption to quit
    f = figure('name','EvoBino SDK : Images and Normalized Depth','NumberTitle','off','keypressfcn','close');
        
    % Setup runtime parameters
    grab_parameters.do_rectify = 1;
    grab_parameters.depth_mode = 0;
    grab_parameters.calc_disparity = 1;
    grab_parameters.calc_distance = 1;
    
    ok = 1;
    % loop over frames
    while ok        
        % grab the current image and compute the depth
        result = mexLeadSense('grab', grab_parameters);
        
        if(strcmp(result, 'OK'))
            % retrieve left image
            image_left = mexLeadSense('retrieveImage', 0); %left
            % retrieve right image
            image_right = mexLeadSense('retrieveImage', 1); %right

            % retrieve normalized distance z color
            image_distance_z_color = mexLeadSense('retrieveNormalizedDepth', 1); %distance z color
            % retrieve the real distance z
            distance_z = mexLeadSense('retrieveDepth', 0); %distance

            % display
            subplot(2,2,1);
            imshow(image_left);
            title('Image Left');
            subplot(2,2,2);
            imshow(image_right);
            title('Image Right');
            subplot(2,2,3);
            imshow(image_distance_z_color);
            title('Distance Z (Color)')
            subplot(2,2,4);
            % Compute the depth histogram
            val_ = find(isfinite(distance_z(:))); % handle wrong depth values
            distance_z_filtered = distance_z(val_);
            [bincounts] = histc(distance_z_filtered(:), binranges);
            bar(binranges, bincounts, 'histc');
            title('Distance Z histogram');
            xlabel('mm');
        end

        drawnow; %this checks for interrupts
        ok = ishandle(f); %does the figure still exist
    end
end

% Make sure to call this function to free the memory before use this again
mexLeadSense('close');
close all;
clear mex;
disp('Exit');