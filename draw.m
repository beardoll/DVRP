function [] = draw()
    drawCircle(30);
    hold on;
    drawCircle(60);
    drawCircle(150);
    drawLine(0,150);
    drawLine(60,150);
    drawLine(120,150);
    drawLine(180,150);
    drawLine(240,150);
    drawLine(300,150);
    [staticCustomer, dynamicCustomer, store] = readxml();
    for i = 1:length(staticCustomer)
        x = staticCustomer(i).cx;
        y = staticCustomer(i).cy;
        plot(x, y, 'r*');
    end
    for i = 1:length(dynamicCustomer)
        x = dynamicCustomer(i).cx;
        y = dynamicCustomer(i).cy;
        plot(x, y, 'kv');
    end
    for i = 1:length(store)
        x = store(i).cx;
        y = store(i).cy;
        plot(x, y, 'gd');
    end
    hold off;
    axis([-150, 150, -150, 150]);
end

function [] = drawCircle(r)  
    theta=0:0.05:(2*pi+0.05);  
    Circle1=r*cos(theta);  
    Circle2=r*sin(theta);  
    c=[123,14,52];  
    plot(Circle1,Circle2,'m','linewidth',1);   
end  

function [] = drawLine(theta, r)
    % ?(0, 0)????theta?????????r???
    % theta??
    theta = 2*pi*theta/360;
    delta_r = 0:0.1:(r+0.1);
    x = delta_r * cos(theta);
    y = delta_r * sin(theta);
    plot(x, y, 'c', 'linewidth', 1);
end

