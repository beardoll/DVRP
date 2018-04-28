function [] = hg()
    [staticCustomer, dynamicCustomer, store] = readxml();
    allCustomer = [staticCustomer, dynamicCustomer];
    customerNum = length(allCustomer);
    customerDist = [];
    for i = 1:customerNum-1
        for j = i+1:customerNum
            if(i == j) 
                continue
            else
                d = sqrt((allCustomer(i).cx - allCustomer(j).cx)^2 + ...
                    (allCustomer(i).cy - allCustomer(j).cy)^2);
                customerDist = [customerDist, d];
            end
        end
    end
    customerDist = sort(customerDist);
    storeDist = [];
    storeNum = length(store);
    for i = 1:storeNum-1
        for j = i+1:storeNum
            if(i == j)
                continue
            else
                d = sqrt((store(i).cx - store(j).cx)^2 + ...
                    (store(i).cy - store(j).cy)^2);
                storeDist = [storeDist, d];
            end
        end
    end
    storeDist = sort(storeDist);
    customer2storeDist = [];
    for i = 1:customerNum
        for j = 1:storeNum
            d = sqrt((allCustomer(i).cx - store(j).cx)^2 + ...
                  (allCustomer(i).cy - store(j).cy)^2);
            customer2storeDist = [customer2storeDist, d];
        end
    end
    customer2storeDist = sort(customer2storeDist);
    %figure(1)
    %hist(customerDist, 10);
end