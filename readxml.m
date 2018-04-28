function [staticCustomer, dynamicCustomer, store] = readxml()
    clear;
    xmlDoc = xmlread('bench_exp.xml');
    staticCustomer = extractSpotInfo(xmlDoc, 'staticCustomer');
    dynamicCustomer = extractSpotInfo(xmlDoc, 'dynamicCustomer');
    store = extractSpotInfo(xmlDoc, 'store');
end 


function [spotSet] = extractSpotInfo(xmlDoc, rootTagName)
    % rootTagName: staticCustomer, dynamicCustomer或者store等顶级节点
    staticCustomerNode = xmlDoc.getElementsByTagName(rootTagName).item(0);
    childNode = staticCustomerNode.getFirstChild;

    spotSet = [];
    while ~isempty(childNode)
        if childNode.getNodeType == childNode.ELEMENT_NODE 
            node.id = str2double(char(childNode.getAttribute('id')));
            subChildNode = childNode.getFirstChild;
            while ~isempty(subChildNode)
                if subChildNode.getNodeType == subChildNode.ELEMENT_NODE
                    tagName = char(subChildNode.getTagName);
                    if strcmp(tagName, 'cx') == 1 
                        node.cx = str2double(char((subChildNode.getFirstChild.getData)));
                    elseif strcmp(tagName, 'cy') == 1
                        node.cy = str2double(char((subChildNode.getFirstChild.getData)));
                    elseif strcmp(tagName, 'quantity') == 1
                        node.quantity = str2double(char((subChildNode.getFirstChild.getData)));
                    elseif strcmp(tagName, 'startTime') == 1
                        node.startTime = str2double(char((subChildNode.getFirstChild.getData)));
                    elseif strcmp(tagName, 'endTime') == 1
                        node.endTime = str2double(char((subChildNode.getFirstChild.getData)));
                    elseif strcmp(tagName, 'arrivedTime') == 1
                        node.arrivedTime = str2double(char((subChildNode.getFirstChild.getData)));
                    elseif strcmp(tagName, 'tolerantTime') == 1
                        node.tolerantTime = str2double(char((subChildNode.getFirstChild.getData)));
                    elseif strcmp(tagName, 'serviceTime') == 1
                        node.serviceTime = str2double(char((subChildNode.getFirstChild.getData)));
                    elseif strcmp(tagName, 'choice') == 1
                        node.choice = str2double(char((subChildNode.getFirstChild.getData)));
                    else
                        fprintf('Not known node element\n');
                    end
                end
                subChildNode = subChildNode.getNextSibling;
            end
            spotSet = [spotSet, node];
        end
        childNode = childNode.getNextSibling;
    end
end
