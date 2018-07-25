clc;
disp('### EvoBinoSDK With Matlab (Point Cloud) ###');
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
    
    % Set Confidence Threshold (70%)
    mexLeadSense('setConfidenceThreshold', 0.3);
    
    % Define maximum distance z (unit: mm)
    maxDistance = 6000;
    mexLeadSense('setDistanceMaxValue', maxDistance);
        
    % Create Figure and wait for keyboard interruption to quit
    f = figure('name','EvoBino SDK : Point Cloud','NumberTitle','off','keypressfcn','close');
    
    % Setup runtime parameters
    grab_parameters.do_rectify = 1;
    grab_parameters.depth_mode = 0;
    grab_parameters.calc_disparity = 1;
    grab_parameters.calc_distance = 1;
    
    ok = 1;
    player = pcplayer([-maxDistance maxDistance], [-maxDistance maxDistance], [0 maxDistance]);
    % loop over frames
    while (ok && isOpen(player))   
        % grab the current image and compute the depth
        result = mexLeadSense('grab', grab_parameters);
        
        if(strcmp(result, 'OK'))
            % retrieve left view
            view_left = mexLeadSense('retrieveView', 0); %left view
            % display
            imshow(view_left);
        
            % retrieve point cloud
            pt = mexLeadSense('retrieveDepth', 10); %DEPTH_TYPE_DISTANCE_XYZ
            pc = pointCloud(pt);
            pc.Color = view_left;
            % down sample
            pc_new = pcdownsample(pc, 'gridAverage', 20); 
            % display
            view(player, pc_new); 
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