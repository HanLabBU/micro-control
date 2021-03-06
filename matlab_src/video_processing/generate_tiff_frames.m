%% using the cell generated by file_frames_for_step_size and the names of the files to load
function [data, info] = generate_tiff_frames(filecell, frames)
if all(class(filecell) == 'char') % convert to cell if not already a cell
    filecell = {filecell};
end

% acquire total number of frames in the video. Should be 2000 in most cases
nframes = sum(cellfun(@length,frames));

% assert
if nframes > 2000
    warning('Using more than 2000 frames');
end

% initialize output structures: data to hold the frame data, info to hold
% the information corresponding to that frame
data = zeros(1024,1024,nframes,'uint16'); % used this line from hua-an and kyle's "tiff2matrix"
info = [];
position = 1;

for f=1:numel(frames) % for each of the files in our video
    if isempty(frames{f}) % if there are no frames from here that we want, continue
        continue
    else
        currinfo = imfinfo(filecell{f}); % get file information for this whole tiff file. % used this line from hua-an and kyle's "tiff2matrix"
        for i=1:numel(frames{f})
            currframe = frames{f}(i);
            info = cat(1,info,currinfo(currframe));
            data(:,:,position) = imread(filecell{f},'Index',currframe,'Info',currinfo); % used this line from hua-an and kyle's "tiff2matrix"
            position = position + 1;
        end
    end
    
end

end