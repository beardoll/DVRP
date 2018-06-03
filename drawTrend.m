function [] = drawTrend()
    txtFilename = 'pr06.txt';
    [index, currentCost, globalCost] = textread(txtFilename, '%f %f %f');
    % xaxis = 1000:1000:15000;
    % xaxis = [13, xaxis];
    xaxis = [13:15000];
    plot(index(xaxis), currentCost(xaxis), 'b-', 'LineWidth', 1);
    hold on
    plot(index(xaxis), globalCost(xaxis), 'r-', 'LineWidth', 2);
    % plot(index(1:12), 6000*ones(1,12), 'gx', 'MarkerSize', 16)
    axis([0 14999 3000 6000])
    legend('当前接受的解', '当前最优解')
    xlabel('迭代次数')
    ylabel('路径代价')
    hold off
end