function [] = saveMediumFile()
    systemName = 'DVRPSimulation';
    expName = 'DynamicismExperiment';
    config = getConfig(systemName, expName);
    for i = 1: length(config.setName)
        for j = 1: length(config.methods)
            root_path = fullfile('./data', systemName, expName, char(config.setName(i)));
            sub_path = char(config.methods(j));
            saveResultInADir(root_path, sub_path, config.capacity);            
        end
    end
end

function [servedCustomerNum, usedQuantity] = parse(routeSet)
    % 获取routeSet中的顾客节点数（也就是已经服务的）以及货车的平均使用率
    % routeSet: cell数组，每个元素是一个spotSet，不包括depot节点
    servedCustomerNum = 0;
    usedQuantity = [];
    for i = 1: length(routeSet)
        currentRoute = routeSet{i};
        quantitySum_ = 0;
        for j = 1: length(currentRoute)
            node = currentRoute(j);
            if node.type == 'C' || node.type == 'P'
                quantitySum_ = quantitySum_ + node.quantity;
                servedCustomerNum = servedCustomerNum + 1;
            end
        end
        usedQuantity = [usedQuantity, quantitySum_];
    end
end

function saveResultInADir(root_path, sub_path, capacity)
    % 保存一个文件夹下的数据（从属于:某个集合-->某个Config方案）
    % path: 文件夹的路径，以当前文件为根目录
    % capacity: 货车容量
    
    files = dir(fullfile(root_path, sub_path, 'xml', '*.xml'));
    avgRejectCustomerNum = 0;
    avgServedCustomerNum = 0;
    avgVehicleNum = 0;
    avgUsageRatio = 0;
    avgTravelDistance = 0;
    for i = 1:length(files)
        filename = fullfile(root_path, sub_path, 'xml', files(i).name);
        result = readxml(filename, 'readResult');
        routeSet = result.routeSet;
        rejectCustomer = result.rejectCustomer;
        dynamicCustomer = result.dynamicCustomer;
        travelDistance = result.travelDistance;
        [servedCustomerNum, usedQuantity] = parse(routeSet);
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
    if ~exist(fullfile(root_path, 'summary'))
        mkdir(fullfile(root_path, 'summary'));
    end
    savedPath = fullfile(root_path, 'summary', strcat(sub_path, '.mat'));
    save(savedPath, 'avgRejectCustomerNum', 'avgServedCustomerNum', 'avgVehicleNum', ...
       'avgUsageRatio', 'avgTravelDistance');
end