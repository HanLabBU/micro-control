% first get tiff file names

[fi,dir] = uigetfile('*.tif','multiselect','on');
for f=1:numel(fi)
    fi{f} = [dir fi{f}];
end
save('tonepuff_file_names.mat','fi');

%%
clear
load('tonepuff_file_names.mat');
motion_correct_mike_v1(fi,'tonepuff_102618_1752_',[0 nan],'all'); % will leave last, single frame alone

%%
metadata.suffix = 'tonepuff_102618_1752';
[fi,dir] = uigetfile('*.tif','multiselect','on');


for f=1:numel(fi)
    fi{f} = [dir fi{f}];
end
fi2 = order_filenames_processed(fi);
metadata.tiffs = fi2;



save('tonepuff_102618_1752.mat','metadata');
%%
clear
load('tonepuff_102618_1752.mat')

%%
roi_simon_new(metadata);


%%
load('micro-control-data/processed-data/roi_simon_tonepuff_102618_1752.mat')
load('micro-control-data/processed-data/imgDiff_simon_tonepuff_102618_1752.mat')
roi_overlay(roi_simon, imgDiff_simon)
print('figures/roi_overlay_tonepuff.svg','-dsvg');
%%
roi_overlay('',imgDiff_simon);
print('figures/max_minus_mean_tonepuff.svg','-dsvg');



%% maybe redo that imaging session?
x = csvread('tonepuff_withmouse_v4_102618.txt',0,0);
load('micro-control-data/processed-data/trace_simon_tonepuff_102618_1752.mat')

tone_onset = [0; diff(x(:,6)) == 1];
tone_offset = [diff(x(:,6)) == -1; 0];
puff_onset = [0; diff(x(:,4)) == 1];
fluor = cat(1,trace_simon.trace);
dff = bsxfun(@rdivide,bsxfun(@minus,fluor,mean(fluor,2)),mean(fluor,2));

mvmt_triggered = movementTriggeredPeak(dff',tone_onset,50);
mean_mvmt_triggered = (squeeze(mean(mvmt_triggered,2))');

%% now sort
inds = 65:69;
mn = mean(mean_mvmt_triggered(:,inds),2);
[~,i] = sort(mn);
mean_mvmt_triggered_sort = mean_mvmt_triggered(i,:);
imagesc(-50:50,1:size(mean_mvmt_triggered_sort,1),mean_mvmt_triggered_sort);
colormap(jet)
caxis([-0.1 0.2])
colorbar
print('figures/tonepuff_aligned_to_tone.svg','-dsvg');
%%