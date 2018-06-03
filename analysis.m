function [] = analysis()
    % ********************** 初始化config ************************ %
    systemName = 'O2OSimulation';
    expName = 'VariousSPRExperiment';
    % systemName = 'DVRPSimulation';
    % expName = 'DynamicismExperiment';
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
            avgRejectCustomerNum(i, j) = data.avgRejectCustomerNum/config.customerNum(i);
            avgServedCustomerNum(i, j) = data.avgServedCustomerNum/config.customerNum(i);
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
    if ~config.grid
        figure(1)
    else
        subplot(2, 2, 1)
    end
    for i = 1: lineNum
        plot(xAxis, avgServedCustomerNum(:, i), strcat(colors(i), 'o-'), ...
            'LineWidth', 1.5, 'MarkerSize', 8);
        hold on
    end
    hold off
    if config.enlarge
        legend(config.legend, 'FontSize', 20)
    else
        legend(config.legend)
    end
    set(gca, 'XTick', 1:1:4)
    set(gca, 'XTickLabel', config.xTickLabel)
    if ~isempty(config.axis1)
        axis(config.axis1)
    end
    if ~isempty(config.yLabel)
        ylabel(config.yLabel(1), 'FontSize',24)
    end
    if strcmp(config.xLabel, '') ~= 1
        xlabel(config.xLabel, 'FontSize',24)
    end
    if config.with_title
        title('服务成功率')
    end
    
    % ********************** travelDistance ************************ %
    if ~config.grid
        figure(2)
    else
        subplot(2, 2, 2)
    end
    for i = 1: lineNum
        plot(xAxis, avgTravelDistance(:, i), strcat(colors(i), 'o-'), ...
            'LineWidth', 1.5, 'MarkerSize', 8);
        hold on
    end
    hold off
    if config.enlarge
        legend(config.legend, 'FontSize', 20)
    else
        legend(config.legend)
    end
    set(gca, 'XTick', 1:1:4)
    set(gca, 'XTickLabel', config.xTickLabel)
    if ~isempty(config.axis2)
        axis(config.axis2)
    end
    if ~isempty(config.yLabel)
        ylabel(config.yLabel(2), 'FontSize',24)
    end
    if strcmp(config.xLabel, '') ~= 1
        xlabel(config.xLabel, 'FontSize',24)
    end
    if config.with_title
        title('服务每个顾客的平均代价')
    end
    
    % ********************** vehicleNum ************************ %
    if ~config.grid
        figure(3)
    else
        subplot(2, 2, 3)
    end
    for i = 1: lineNum
        plot(xAxis, avgVehicleNum(:, i), strcat(colors(i), 'o-'), ...
            'LineWidth', 1.5, 'MarkerSize', 8);
        hold on
    end
    hold off
    if config.enlarge
        legend(config.legend, 'FontSize', 20)
    else
        legend(config.legend)
    end
    set(gca, 'XTick', 1:1:4)
    set(gca, 'XTickLabel', config.xTickLabel)
    if ~isempty(config.axis3)
        axis(config.axis3)
    end
    if ~isempty(config.yLabel)
        ylabel(config.yLabel(3), 'FontSize',24)
    end
    if strcmp(config.xLabel, '') ~= 1
        xlabel(config.xLabel, 'FontSize',24)
    end
    if config.with_title
        title('服务每个顾客平均车辆数')
    end
    
    % ********************** usageRatio ************************ %
    if ~config.grid
        figure(4)
    else
        subplot(2, 2, 4)
    end
    for i = 1: lineNum
        plot(xAxis, avgUsageRatio(:, i), strcat(colors(i), 'o-'), ...
            'LineWidth', 1.5, 'MarkerSize', 8);
        hold on
    end
    hold off
    if config.enlarge
        legend(config.legend, 'FontSize', 20)
    else
        legend(config.legend)
    end
    set(gca, 'XTick', 1:1:4)
    set(gca, 'XTickLabel', config.xTickLabel)
    if ~isempty(config.axis4)
        axis(config.axis4)
    end
    if ~isempty(config.yLabel)
        ylabel(config.yLabel(4), 'FontSize',24)
    end
    if strcmp(config.xLabel, '') ~= 1
        xlabel(config.xLabel, 'FontSize',24)
    end
    if config.with_title
        title('货车的平均利用率')
    end
end