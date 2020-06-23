# New-Haven-sources
Partial source and header files for New Haven game board map.  

## Introduction
<img class="ui medium rounded centered image" src="../images/mino/new_haven.png"> 

New Haven is a board game we were tasked with recreating using C++ for COMP 345 (Advanced Program Design with C++) at Concordia University in the semester of Winter 2020. **This writeup focuses only on the creation of the game board via parsing a custom map file, creating a graph structure, and performing a traversal to calculate points in terms of the resources collected on the map.** This all encompases roughly 800 lines of code, as requested. An important note, the project required us to use raw pointers for all class data members (including primitive types), so some of the references may seem unneccesarily complex. Also, functionalties like exception-handling, input guards, and memory management need to be reworked or implemented.

*And without further ado...*

## Relevant header and source files
* GBMapLoader.cpp, GBMapLoader.h
* GBMap.cpp, GBMap.h
* Graph.cpp, Graph.h
* Node.cpp, Node.h
* TileNode.cpp, TileNode.h
* Resources.cpp, Resources.h

*Roughly 800 lines of code including empty lines and comments.*

## Overview

<img class="ui fluid rounded centered image" src="../images/mino/ClassDiagram.png">

## Parsing the .gbmap file
Below is a sample of a custom map file, `test.gbmap`:

```
# Length and height of the grid
LENGTH	3
HEIGHT	3

# Row 0, Column 0
RESOURCE	0	STONE	SHEEP	TIMBER	WHEAT

# Row 1, Column 2
RESOURCE	5	TIMBER	WHEAT	WHEAT	TIMBER

# Row 2, Column 2
DISABLE		8
```

The above gbmap file will result in the gameboard shown below. Note the order of the resources from left-to-right in the gbmap file and how they correspond to the resources on the tile. 

<img class="ui medium rounded centered image" src="../images/mino/test_gbmap.png"> 

The parsing is done with a while-loop that iterates through the file line-by-line. The loop is continued if an empty or whitespaced line is detected. If no such line is detected, then it is tokenized and the first token is compared in an if-else block where matching certain keywords will determine in which containers the data is stored.  Lines that do not start with any of the keywords are simply ignored and this behaviour can be used to include comments as is done in the test file with the '#' symbol.  

```cpp
while (inFile) {
	getline(inFile, lineRead);

	bool whiteSpaced = true;
	for (int i = 0; i < lineRead.length(); i++)
		if (!isspace(lineRead.at(i))) {
			whiteSpaced = false;
			break;
		}

	if (lineRead.empty() || whiteSpaced)
		continue;

	stringstream strstr(lineRead);
	istream_iterator<string> it(strstr);
	istream_iterator<string> end; 
	vector<string> results(it, end);

	if (results[0].compare("LENGTH") == 0) 
		length = std::stoi(results[1]);
	else if (results[0].compare("HEIGHT") == 0) 
		height = std::stoi(results[1]);	
	else if (results[0].compare("RESOURCE") == 0){
		resourceData[std::stoi(results[1])] = { strToEnum(results[2]), strToEnum(results[3]), strToEnum(results[4]), strToEnum(results[5]) };
		resourceIndices.push_back(std::stoi(results[1]));
	}
	else if (results[0].compare("DISABLE") == 0) {
		disableData.push_back(std::stoi(results[1]));
	}
}
inFile.close();
```

## Creating and linking the two graphs

In `GBMapLoader.cpp`, the length and height values are read from the map file are passed to the function `Graph::makeGridGraph`. Note that there are 4 resource nodes to ever tile node, so the length and height are both multiplied by 2 for the resource graph.  

```cpp
gb_map.getTileGraph()->makeGridGraph(length, height, NodeType::TILE);
gb_map.getResourceGraph()->makeGridGraph(length * 2, height * 2, NodeType::RESOURCE);
```

In `Graph.cpp`, function `Graph::makeGridGraph` creates the nodes which are unconnected at first, but the for-loop and if-statements below ensure that edges pointing to other nodes are added to create a connected grid. Each of the four if-statements is responsible for one of four directions: up, down, left, right, respectively. The conditional logic ensures the creation of the relevant connections on the "inside" of the grid, while leaving all outward pointing edges along the perimeter of the grid as the default value of `nullptr` (i.e. the edge doesn't exist).  

```cpp
for (int i = 0; i < totalNodes; i++)
{
	if (i - length >= 0)
		nodes[0][i]->addEdge(nodes[0][i - length], Direction::UP);
	if (i + length <= totalNodes - 1)
		nodes[0][i]->addEdge(nodes[0][i + length], Direction::DOWN);
	if (i - 1 >= 0 && i % length != 0)
		nodes[0][i]->addEdge(nodes[0][i - 1], Direction::LEFT);
	if (i + 1 <= totalNodes - 1 && (i+1)%length != 0)			
		nodes[0][i]->addEdge(nodes[0][i + 1], Direction::RIGHT);
}
```

The above processes result in two isolated graphs, one for tiles and one for resources, respectively, as visualized below. 

<img class="ui large rounded centered image" src="../images/mino/tile_nodes.png">  

<img class="ui large rounded centered image" src="../images/mino/resource_nodes.png"> 

The challenge now is to link the corresponding tile nodes to their respective cluster of 4 resource nodes. This correspondence is highlighted by the image directly above with the checkered pattern coloring of the resource nodes and the corresponding tile node ID in blue. In order for this to be done dynamically for any grid size and shape, we need to use the formulas below.  

<img class="ui large rounded centered image" src="../images/mino/equation.png">  

The equations above are implemented in the function `Graph::linkResourceNodes` using a for-loop on each of the tile nodes as shown below.  

```cpp
for (int i = 0; i < totalNodes; i++)
{
	if (rowCount == *this->length) {
		rowCount = 0; 
		rowValue++;
	}
 
	int firstResource = (i * 2) + (*this->length*rowValue*2);

	static_cast<TileNode*>(nodes[0][i])
		->linkResourceNode(static_cast<Resource*>(resourceGraph->getNode(firstResource)), 0);
	static_cast<TileNode*>(nodes[0][i])
		->linkResourceNode(static_cast<Resource*>(resourceGraph->getNode(firstResource + 1)), 1);
	static_cast<TileNode*>(nodes[0][i])
		->linkResourceNode(static_cast<Resource*>(resourceGraph->getNode(firstResource + *this->length * 2)), 2);
	static_cast<TileNode*>(nodes[0][i])
		->linkResourceNode(static_cast<Resource*>(resourceGraph->getNode(firstResource + *this->length * 2 + 1)), 3);

	rowCount++; 
}
```

The two graphs combined result in a three-dimensional grid graph that resembles a trapazoidal prism, as shown below. For simplicity, a 2x2 tile grid graph is shown instead of the 3x3 example from `test.gbmap`.  

<img class="ui fluid rounded centered image" src="../images/mino/2x2_grid_visual.png">

## Calculating harvested resources using conditional DFS
In New Haven, players accumulate resources to construct buildings in their villages. Resources are accumulated by placing a harvest tile onto the game board. By matching adjacent resources, they are able to create chains of accumulating resources. In the example below, the player placed a harvest tile at row 2, column 2 (highlighted in blue). This results in an accumulation of 12 timber (red) and 4 sheep (green). 

<img class="ui medium rounded centered image" src="../images/mino/adj.png">  

Our graph structure comes in handy here. In `GBMap.cpp`, the function `GBMap::calcResourceAdjacencies` will perform a DFS traversal starting on each of the resource nodes on the placed harvest tile. Once the traversal is complete, a count is done for each of the resource types to determine the amount accumulated. Once the amounts are recorded, the `visited` state of all the nodes are reset.   

```cpp
void GBMap::calcResourceAdjacencies(TileNode* root, std::map<ResourceType, int> &output)
{
	resourceGraph->DFS_ByType(root->getResourceNodes()[0]);
	resourceGraph->DFS_ByType(root->getResourceNodes()[1]);
	resourceGraph->DFS_ByType(root->getResourceNodes()[2]);
	resourceGraph->DFS_ByType(root->getResourceNodes()[3]);

	for (int i = 0; i < resourceGraph->getNumEnabledNodes(); i++) {
		if (resourceGraph->getNode(i)->isVisited()) {
			switch (static_cast<Resource*>(resourceGraph->getNode(i))->getType()) {

			case ResourceType::SHEEP:
				output[ResourceType::SHEEP]++;
				break;
			case ResourceType::STONE:
				output[ResourceType::STONE]++;
				break;
			case ResourceType::WHEAT:
				output[ResourceType::WHEAT]++;
				break;
			case ResourceType::TIMBER:
				output[ResourceType::TIMBER]++;
				break;
			}
		}
	}
	resourceGraph->resetAllVisited();
}
```

In `Graph.cpp`, the function `Graph::DFS_ByType`, shown below, peforms a conditional traversal, i.e. it continues to traverse so long as:  
* The edge exists i.e. is not `nullptr`. 
* The node has not been visited.
* The node is enabled.
* The node shares the same resource type. 

```cpp
void Graph::DFS_ByType(Resource* node) {
	*node->visited = true;

	if (node->up && !*node->up->visited && *node->up->enabled && (static_cast<Resource*>(node->up)->getType() == node->getType()))
		DFS_ByType(static_cast<Resource*>(node->up));

	if (node->down && !*node->down->visited && *node->down->enabled && (static_cast<Resource*>(node->down)->getType() == node->getType()))
		DFS_ByType(static_cast<Resource*>(node->down));

	if (node->left && !*node->left->visited && *node->left->enabled && (static_cast<Resource*>(node->left)->getType() == node->getType()))
		DFS_ByType(static_cast<Resource*>(node->left));

	if (node->right && !*node->right->visited && *node->right->enabled && (static_cast<Resource*>(node->right)->getType() == node->getType()))
		DFS_ByType(static_cast<Resource*>(node->right));
}
```