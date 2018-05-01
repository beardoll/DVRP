function [] = saveMediumFile()
    systemName = 'O2OSimulation';
    setName = {'small_set', 'medium_set', 'larger_set', 'largest_set'};
    methods = {'replan_sampling_evaluation', 'replan_sampling_random', ... 
        'replan_no_sampling', 'no_replan_sampling_evaluation'};
    for i = 1: length(setName)
        for j = 1: length(methods)
            root_path = fullfile('./data', systemName, char(setName(i)));
            sub_path = char(methods(j));
            saveResultInADir(root_path, sub_path);            
        end
    end
end

function [servedCustomerNum, usedQuantity] = analysis(routeSet)
    % routeSet: cell数组，每个元素是一个spotSet，不包括depot节点
    servedCustomerNum = 0;
    usedQuantity = [];
    for i = 1: length(routeSet)
        currentRoute = routeSet{i};
        quantitySum_ = 0;
        for j = 1: length(currentRoute)
            node = currentRoute(j);
            if node.type == 'C'
                quantitySum_ = quantitySum_ + node.quantity;
                servedCustomerNum = servedCustomerNum + 1;
            end
        end
        usedQuantity = [usedQuantity, quantitySum_];
    end
end

function saveResultInADir(root_path, sub_path)
    % 保存一个文件夹下的数据（从属于:某个集合-->某个Config方案）
    % path: 文件夹的路径，以当前文件为根目录
    
    %*********** 全局变量 *************%
    capacity = 30;
    %*********************************%
    
    files = dir(fullfile(root_path, sub_path, 'xml', '*.xml'));
    avgRejectCustomerNum = 0;
    avgServedCustomerNum = 0;
    avgVehicleNum = 0;
    avgUsageRatio = 0;
    avgTravelDistance = 0;
    for i = 1:length(files)
        filename = fullfile(root_path, sub_path, 'xml', files(i).name);
        [routeSet, rejectCustomer, dynamicCustomer, travelDistance] = readxml(filename);
        [servedCustomerNum, usedQuantity] = analysis(routeSet);
        avgRejectCustomerNum = avgRejectCustomerNum + length(rejectCustomer);
        avgServedCustomerNum = avgServedCustomerNum + servedCustomerNum;
        avgVehicleNum = avgVehicleNum + length(routeSet);
        avgUsageRatio = avgUsageRatio + sum(usedQuantity) / length(routeSet) / capacity;
        avgTravelDistance = avgTravelDistance + travelDistance;
    end
    avgRejectCustomerNum = avgRejectCustomerNum / length(files);
    avgServedCustomerNum = avgServedCustomerNum / length(files);
    avgVehicleNum = avgVehicleNum / length(files);
    avgUsageRatio = avgUsageRatio / length(files);
    avgTravelDistance = avgTravelDistance / length(files);
    savedPath = fullfile(root_path, 'summary', strcat(sub_path, '.mat'));
    save(savedPath, 'avgRejectCustomerNum', 'avgServedCustomerNum', 'avgVehicleNum', ...
        'avgUsageRatio', 'avgTravelDistance');
end