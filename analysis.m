function [] = analysis()
    % ********************** 初始化config ************************ %
    % systemName = 'O2OSimulation';
    % expName = 'VariousTWLExperiment';
    systemName = 'DVRPSimulation';
    expName = 'DynamicismExperiment';
    config = getConfig(systemName, expName);
    % *********************************************************** %
    % 各变量定义
    avgRejectCustomerNum = zeros(length(config.setName), length(config.methods));
    avgServedCustomerNum = zeros(length(config.setName), length(config.methods));
    avgVehicleNum = zeros(length(config.setName), length(config.methods));
    avgUsageRatio = zeros(length(config.setName), length(config.methods));
    avgTravelDistance = zeros(length(config.setName), length(config.methods));
    for i = 1: length(config.setName)
        root_path = fullfile('./data', systemName, expName, char(config.setName(i)));
        for j = 1: length(config.methods)
            filename = fullfile(root_path, 'summary', strcat(char(config.methods(j)), '.mat'));     
            data = load(filename);
            avgRejectCustomerNum(i, j) = data.avgRejectCustomerNum/config.customerNum;
            avgServedCustomerNum(i, j) = data.avgServedCustomerNum/config.customerNum;
            avgVehicleNum(i, j) = data.avgVehicleNum / data.avgServedCustomerNum;
            avgUsageRatio(i, j) = data.avgUsageRatio;
            avgTravelDistance(i, j) = data.avgTravelDistance / data.avgServedCustomerNum;        
        end
    end
    % 作图
    draw(avgServedCustomerNum, avgTravelDistance, avgVehicleNum, avgUsageRatio, config)
end

function [] = draw(avgServedCustomerNum, avgTravelDistance, avgVehicleNum, avgUsageRatio, config)
    colors = ['r', 'b', 'g', 'k', 'm', 'y'];
    xAxis = 1:4;
    lineNum = size(avgServedCustomerNum, 2);
    % ********************** servedCustomerNum ************************ %
    subplot(2, 2, 1)
    for i = 1: lineNum
        plot(xAxis, avgServedCustomerNum(:, i), strcat(colors(i), 'o-'), ...
            'LineWidth', 1.5, 'MarkerSize', 8);
        hold on
    end
    hold off
    legend(config.legend)
    set(gca, 'XTick', 1:1:4)
    set(gca, 'XTickLabel', config.xLabel)
    if ~isempty(config.axis1)
        axis(config.axis1)
    end
    title('服务成功率')
    
    % ********************** travelDistance ************************ %
    subplot(2, 2, 2);
    for i = 1: lineNum
        plot(xAxis, avgTravelDistance(:, i), strcat(colors(i), 'o-'), ...
            'LineWidth', 1.5, 'MarkerSize', 8);
        hold on
    end
    hold off
    legend(config.legend)
    set(gca, 'XTick', 1:1:4)
    set(gca, 'XTickLabel', config.xLabel)
    if ~isempty(config.axis2)
        axis(config.axis2)
    end
    title('服务每个顾客的平均代价')
    
    % ********************** vehicleNum ************************ %
    subplot(2, 2, 3);
    for i = 1: lineNum
        plot(xAxis, avgVehicleNum(:, i), strcat(colors(i), 'o-'), ...
            'LineWidth', 1.5, 'MarkerSize', 8);
        hold on
    end
    hold off
    legend(config.legend)
    set(gca, 'XTick', 1:1:4)
    set(gca, 'XTickLabel', config.xLabel)
    if ~isempty(config.axis3)
        axis(config.axis3)
    end
    title('服务每个顾客平均车辆数')
    
    % ********************** usageRatio ************************ %
    subplot(2, 2, 4);
    for i = 1: lineNum
        plot(xAxis, avgUsageRatio(:, i), strcat(colors(i), 'o-'), ...
            'LineWidth', 1.5, 'MarkerSize', 8);
        hold on
    end
    hold off
    legend(config.legend)
    set(gca, 'XTick', 1:1:4)
    set(gca, 'XTickLabel', config.xLabel)
    if ~isempty(config.axis4)
        axis(config.axis4)
    end
    title('货车的平均利用率')
end