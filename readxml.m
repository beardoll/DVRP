function [routeSet, rejectCustomer, dynamicCustomer] = readxml(filename)
    xmlDoc = xmlread(filename);
    routeSet = extractRouteSetInfo(xmlDoc);
    rejectCustomer = extractSpotSetInfo(xmlDoc, 'rejectCustomer');
    dynamicCustomer = extractSpotSetInfo(xmlDoc, 'dynamicCustomer');
end 

function [node] = constructEmptyNode()
    % 构造空的node节点
    node.cx = 0;
    node.cy = 0;
    node.serviceTime = 0;
    node.choice = 0;
    node.startTime = 0;
    node.endTime = 0;
    node.arrivedTime = 0;
    node.tolerantTime = 0;
    node.quantity = 0;
    node.id = 0;
    node.type = 'UNK';
end

function [node] = extractSpotInfo(spotNode)
    % spotNode: Node节点，下辖属性值
    node = constructEmptyNode();
    node.id = str2double(char(spotNode.getAttribute('id')));
    node.type = char(spotNode.getAttribute('type'));
    if node.type == 'S'
        node.quantity = 0;
    end
    propertyNode = spotNode.getFirstChild;
    while ~isempty(propertyNode)
        if propertyNode.getNodeType == propertyNode.ELEMENT_NODE
            tagName = char(propertyNode.getTagName);
            if strcmp(tagName, 'cx') == 1 
                node.cx = str2double(char((propertyNode.getFirstChild.getData)));
            elseif strcmp(tagName, 'cy') == 1
                node.cy = str2double(char((propertyNode.getFirstChild.getData)));
            elseif strcmp(tagName, 'quantity') == 1
                node.quantity = str2double(char((propertyNode.getFirstChild.getData)));
            elseif strcmp(tagName, 'startTime') == 1
                node.startTime = str2double(char((propertyNode.getFirstChild.getData)));
            elseif strcmp(tagName, 'endTime') == 1
                node.endTime = str2double(char((propertyNode.getFirstChild.getData)));
            elseif strcmp(tagName, 'arrivedTime') == 1
                node.arrivedTime = str2double(char((propertyNode.getFirstChild.getData)));
            elseif strcmp(tagName, 'tolerantTime') == 1
                node.tolerantTime = str2double(char((propertyNode.getFirstChild.getData)));
            elseif strcmp(tagName, 'serviceTime') == 1
                node.serviceTime = str2double(char((propertyNode.getFirstChild.getData)));
            elseif strcmp(tagName, 'choice') == 1
                node.choice = str2double(char((propertyNode.getFirstChild.getData)));
            end
        end
        propertyNode = propertyNode.getNextSibling;
    end
end

function [routeSet] = extractRouteSetInfo(xmlDoc)
    % 所有Route节点
    routeNodeArray = xmlDoc.getElementsByTagName('Route');
    routeSet = cell(1, routeNodeArray.getLength);
    for i = 1: routeNodeArray.getLength-1
        currentRouteNode = routeNodeArray.item(i);
        spotNode = currentRouteNode.getFirstChild;
        spotSet = [];
        while ~isempty(spotNode)
            if spotNode.getNodeType == spotNode.ELEMENT_NODE
                node = extractSpotInfo(spotNode);
                if(node.id ~= 0)
                    % 不包含仓库节点
                    spotSet = [spotSet, node];
                end
            end
            spotNode = spotNode.getNextSibling;
        end
        routeSet{i} = spotSet;
    end
end

function [spotSet] = extractSpotSetInfo(xmlDoc, spotSetName)
    % spotSetName: rejectCustomer或者dynamicCustomer
    % 下辖spot node
    spotSetNode = xmlDoc.getElementsByTagName(spotSetName).item(0);
    spotSet = [];
    spotNode = spotSetNode.getFirstChild;
    while ~isempty(spotNode)
        if spotNode.getNodeType == spotNode.ELEMENT_NODE
            node = extractSpotInfo(spotNode);
            spotSet = [spotSet, node];
        end
        spotNode = spotNode.getNextSibling;
    end
end


