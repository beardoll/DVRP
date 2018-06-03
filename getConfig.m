function [config] = getConfig(systemName, expName)
    config.axis1 = [];
    config.axis2 = [];
    config.axis3 = [];
    config.axis4 = [];
    config.xLabel = '';
    config.yLabel = {};
    config.with_title = false;
    config.grid = true;
    config.enlarge = false;
    if(strcmp(systemName, 'O2OSimulation') == 1)
        config.customerNum = [1, 1, 1, 1];
        config.with_title = true;
        % config.legend = {'RSA', 'RSR', 'RNS', 'NSA'};
        config.capacity = 30;
        % config.methods = {'replan_sampling_evaluation', 'replan_sampling_random', ... 
        %                   'replan_no_sampling', 'no_replan_sampling_evaluation'};
        config.legend = {'RSA', 'RNS', 'NSA'};
        config.methods = {'replan_sampling_evaluation', ... 
                          'replan_no_sampling', 'no_replan_sampling_evaluation'};
        if(strcmp(expName, 'IntegratedExperiment') == 1)
            config.xTickLabel = {'1', '2', '3', '4'};
            config.customerNum = [54, 111, 174, 236];
            config.setName = {'small_set', 'medium_set', 'larger_set', 'largest_set'};
        elseif(strcmp(expName, 'DynamicismExperiment') == 1)
            config.xTickLabel = {'0%', '18.4%', '44.3%', '73.0%'};
            config.customerNum = [174, 174, 174, 174]; 
            config.setName = {'highest_set', 'higher_set', 'mid_set', 'low_set'};
        elseif(strcmp(expName, 'VariousTWLExperiment') == 1)
            config.xTickLabel = {'145.5', '164.6', '181.4', '199.2'};
            config.customerNum = [174, 174, 174, 174]; 
            config.setName = {'short_set', 'mid_set', 'longer_set', 'longest_set'};
        elseif(strcmp(expName, 'VariousSPRExperiment') == 1)
            %config.legend = {'RSA', 'RSR', 'NSA'};
            config.xTickLabel = {'10', '20', '30', '40'};
            config.setName = {'low_set', 'mid_set', 'higher_set', 'highest_set'};
            %config.methods = {'replan_sampling_evaluation', 'replan_sampling_random', ... 
            %   'no_replan_sampling_evaluation'};
            config.legend = {'RSA', 'NSA'};
            config.methods = {'replan_sampling_evaluation', 'no_replan_sampling_evaluation'};
            config.customerNum = [174, 174, 174, 174]; 
            config.axis1 = [1 4 0.6 0.75];
            config.axis2 = [1 4 40 60];
            config.axis3 = [1 4 0.2 0.27];
            config.axis4 = [1 4 0.24 0.3];
        end
    elseif(strcmp(systemName, 'DVRPSimulation') == 1)
        % config.legend = {'RSA-p', 'RSA-n', 'RSR-p', 'RSR-n', 'NSA-p', 'NSA-n'};
        % config.methods = {'replan_sampling_evaluation_pos', 'replan_sampling_evaluation_neg', ...
        %                  'replan_sampling_random_pos', 'replan_sampling_random_neg'... 
        %                  'no_replan_sampling_evaluation_pos', 'no_replan_sampling_evaluation_neg'};
        config.legend = {'RSA-p', 'RSA-n', 'NSA-p', 'NSA-n'};
        config.methods = {'replan_sampling_evaluation_pos', 'replan_sampling_evaluation_neg', ...
                          'no_replan_sampling_evaluation_pos', 'no_replan_sampling_evaluation_neg'};
        config.capacity = 200;
        config.customerNum = [100, 100, 100, 100];
        if(strcmp(expName, 'DynamicismExperiment') == 1)
            config.grid = false;
            config.xLabel = '动态顾客比例';
            config.yLabel = {'服务成功率', '平均服务代价', '平均用车数', '平均使用率'};
            config.xTickLabel = {'20%', '40%', '60%', '80%'};
            config.setName = {'low_set', 'mid_set', 'higher_set', 'highest_set'};
        elseif(strcmp(expName, 'VariousRPFExperiment') == 1)
            config.xTickLabel = {'5', '10', '15', '20'};
            config.setName = {'low_set', 'mid_set', 'higher_set', 'highest_set'};
        end
    end
end