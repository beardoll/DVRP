function [] = drawTWL()
    systemName = 'DVRPSimulation';
    expName = 'DynamicismExperiment';
    setName = {'low_set', 'mid_set', 'higher_set', 'highest_set'};
    x = 50:50:350;
    for i = 1:length(setName)
        testPath = fullfile('data', systemName, expName, char(setName(i)), 'bench.xml');
        [result] = readxml(testPath, 'readBench');
        % allCustomer = [result.staticCustomer, result.dynamicCustomer];
        allCustomer = result.dynamicCustomer;
        TWLDistribution = getDistribution(allCustomer);
        % figure(i);
        % hist(TWLDistribution, x);
        mean(TWLDistribution)
        std(TWLDistribution)
    end
end

function [distribution] = getDistribution(allCustomer)
    distribution = [];
    for i = 1: length(allCustomer)
        distribution = [distribution, allCustomer(i).endTime - allCustomer(i).startTime];
    end
end

