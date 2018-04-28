function [x] = possion(lambda)
    x = 0;
    p = 1;
    while(p >= exp(-lambda))
        u = rand;
        p = u * p;
        x = x + 1;
    end
    x = x - 1;
end

function [k] = possion2(mu)
    y = 0;
    k = 0;
    while y <= 1
        u = rand;
        x = exp(-mu * u);
        y = y + x;
        k = k + 1;
    end
    k = k-1;
end