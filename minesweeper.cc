#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

#define MAX_FRAMERATE 60                                // Maximum framerate for the game window
#define MATRIX_SIZE 20                                  // Matrix dimensions will be MATRIX_SIZE x MATRIX_SIZE  (4x4 is the minimum)
#define PERCENTAGE_MINES 0.2                            // % of matrix' cells that will be mines

#define NORMAL_TEXT_COLOR sf::Color(6, 214, 160)        // #06D6A0
#define ENDGAME_TEXT_COLOR sf::Color(100, 171, 227)     // #64ABE3

#define LINE_COLOR sf::Color(200, 160, 95)              // #C8A05F
#define BG_COLOR sf::Color(250, 210, 160)               // #FAD2A0
#define HOVER_COLOR sf::Color(255, 220, 170)            // #FFDCAA
#define CLICKED_SPACE_COLOR sf::Color(255, 230, 180)    // #FFE6B4
#define MARKED_COLOR sf::Color(100, 171, 227)           // #64ABE3

#define PIXEL_FONT_PATH "../fonts/pixel-font.ttf"
#define END_BACKGROUND_TEXTURE_PATH "assets/tileset.png"
#define END_BG_IMG_SIZE sf::Vector2i(100, 56)           // Dimensions of end bg image in pixels
#define END_BG_COLOR sf::Color(255, 230, 180)           // #FFE6B4


enum directions {north, south, west, east};

directions oppositeDir(directions currentDir){
    switch (currentDir){
        case north:
            return south;
            break;
        case south:
            return north;
            break;
        case west:
            return east;
            break;
        default:
            return west;
            break;
    }
}


sf::Vector2f roundPosVector(sf::Vector2f pos){
    return sf::Vector2f{round(pos.x), round(pos.y)};
}


class gameMatrix{
    private:
        sf::RenderWindow* masterWindow;
        unsigned int numDivs;
        float divWidth;
        vector<vector<sf::Vector2f>> gridMatrix;
        vector<vector<sf::Vertex>> linesVector;     // Cannot use a vector of sf::Vertex[2] arrays so we are using a vector of sf::Vertex vector
        sf::Vector2f middlePos;

        vector<vector<int>> mineAmtMatrix;      // -1 will indicate a mine, -2 a user-marked mine
        vector<vector<bool>> markedCellsMatrix;     // true will indicate a cell marked by the user
        vector<vector<bool>> revealedCellMatrix;    // true will indicate a cell where the user has clicked
    public:
        gameMatrix(sf::RenderWindow &master, unsigned int numDivs);
        vector<vector<sf::Vector2f>> getGridMatrix(){return gridMatrix;}
        vector<vector<sf::Vertex>> getLinesVector(){return linesVector;}
        sf::Vector2f getMiddlePos(){return middlePos;}
        sf::Vector2f getDivWidth(){return sf::Vector2f(divWidth, divWidth);}

        sf::Vector2i getMousePosIndex(sf::Vector2i mousePos);
        void generateMineMatrix(sf::Vector2i mouseIndexPos);
        vector<vector<int>> getMineMatrix(){return mineAmtMatrix;}
        vector<vector<bool>> getRevMatrix(){return revealedCellMatrix;}
        bool checkMarking(sf::Vector2i mouseIndexPos);
        int checkAction(sf::Vector2i mouseIndexPos);    // will return numbers depending on what should happen
        void checkRevealed(sf::Vector2i mouseIndexPos);

        int numberOfMines=-1;
        sf::Color lineColor = LINE_COLOR;
        //sf::Vector2f getAdjacentPos(directions dir, sf::Vector2f currentPos);
        
};

gameMatrix::gameMatrix(sf::RenderWindow &master, unsigned int nDivs){
    unsigned int i=0, j=0;
    vector<sf::Vector2f> rowPositions;
    vector<sf::Vertex> currentLine;

    if (MATRIX_SIZE < 4){
        throw invalid_argument("Matrix dimensions cannot be smaller than 4x4");
    }

    // Arguments validation
    if (master.getSize().x != master.getSize().y){
        throw invalid_argument("game is designed for window to be a square");
    }
    masterWindow = &master;

    if (nDivs == 0){
        throw invalid_argument("number of divs is equal to zero");
    }
    numDivs = nDivs;

    if (((float)(masterWindow->getSize().x)) / ((float)numDivs-1) < 5.f){
        throw invalid_argument("number of divs is not small enough, which results in the width of each division being too small");
    }
    if (numDivs <= 2){
        throw invalid_argument("number of divs is too small, it is recommended to be above 10-20");
    }
    divWidth = ((float)(masterWindow->getSize().x)) / ((float)(numDivs));       // numDivs -1

    // Generating matrix of snake's coordinates
    for(i=0; i<numDivs; i++){
        rowPositions.clear();
        for (j=0; j<numDivs; j++){
            rowPositions.push_back(sf::Vector2f(i*divWidth, j*divWidth));
        }
        gridMatrix.push_back(rowPositions);
    }

    // Generating line grid to view the matrix
    // Vertical lines:
    for (i=0; i<gridMatrix[0].size(); i++){
        currentLine.clear();
        currentLine.push_back(sf::Vertex(gridMatrix[0][i], lineColor));
        currentLine.push_back(sf::Vertex(sf::Vector2f(gridMatrix[(gridMatrix.size()-1)][i].x+divWidth, gridMatrix[(gridMatrix.size()-1)][i].y), lineColor));
        linesVector.push_back(currentLine);
    }
    // Horizontal lines
    for (i=0; i<gridMatrix.size(); i++){
        currentLine.clear();
        currentLine.push_back(sf::Vertex(gridMatrix[i][0], lineColor));
        currentLine.push_back(sf::Vertex(sf::Vector2f(gridMatrix[i][gridMatrix[i].size()-1].x, gridMatrix[i][gridMatrix[i].size()-1].y+divWidth), lineColor));
        linesVector.push_back(currentLine);
    }

    // Calculating position of square in the middle of the grid
    // Coordinates of the norhwesternmost point in the square will be returned
    middlePos = gridMatrix[(gridMatrix.size()/2)-1][(gridMatrix[0].size()/2)-1];
    // if the matrix has an even number of rows & columns, the square NW of the 4 ones in the middle will be returned
}

sf::Vector2i gameMatrix::getMousePosIndex(sf::Vector2i mousePos){
    int long unsigned i=0, j=0;
    sf::Vector2i gridIndexPos;      // Will store the column number in the x value and the row number in the y value

    for (i=0; i<gridMatrix.size(); i++){
        if ((gridMatrix[i][0].x <= mousePos.x) && (mousePos.x < (gridMatrix[i][0].x + divWidth))){
            gridIndexPos.x = i;
        }
    }
    for (i=0; i<gridMatrix[0].size(); i++){
        if ((gridMatrix[0][i].y <= mousePos.y) && (mousePos.y < (gridMatrix[0][i].y + divWidth))){
            gridIndexPos.y = i;
        }
    }

    return gridIndexPos;
}

void gameMatrix::generateMineMatrix(sf::Vector2i mouseIndexPos){
    // Generating mine matrix
    vector<int> tempVector;
    vector<vector<int>> tempMatrix;

    vector<bool> tempVectorBool;
    vector<vector<bool>> tempMatrixBool;

    srand(time(0));

    int nMines=int((gridMatrix.size() * gridMatrix[0].size() + 1)*PERCENTAGE_MINES);
    numberOfMines = nMines+1;

    int long unsigned i=0, j=0;
    int randomX=0, randomY=0;

    // Fills the matrix with zeros so that it has the desired size and we can work with indices (using [] brackets)
    for (i=0; i<gridMatrix.size(); i++){
        tempVector.clear();
        for (j=0; j<gridMatrix[i].size(); j++){
            tempVector.push_back(0);
        }
        tempMatrix.push_back(tempVector);
    }

    // Generates a fixed (PERCENTAGE_MINES) % of mines in relation with the number of cells

    //randomY > mouseIndexPos.y+1 && andomY < mouseIndexPos.y-1
    for (i=0; i<=nMines; i++){
        do{
            randomY = rand()%(gridMatrix.size());
            randomX = rand()%(gridMatrix[0].size());
        }while(((randomY <= mouseIndexPos.y+1 && randomY >= mouseIndexPos.y-1) && (randomX <= mouseIndexPos.x+1 && randomX >= mouseIndexPos.x-1) || (tempMatrix[randomY][randomX] == -1)));

        tempMatrix[randomY][randomX] = -1;
    }

    // Loop that increments the number on each matrix square around a mine
    for (i=0; i<tempMatrix.size(); i++){
        for (j=0; j<tempMatrix[i].size(); j++){
            if (tempMatrix[i][j] == -1){

                if (i>0){   // top 3 squares above mine (NW, N, NE)
                    if (tempMatrix[i-1][j] != -1){
                        tempMatrix[i-1][j] = tempMatrix[i-1][j]+1;
                    }
                    if (j>0){
                        if (tempMatrix[i-1][j-1] != -1){
                            tempMatrix[i-1][j-1] = tempMatrix[i-1][j-1]+1;
                        }
                    }
                    if (j<(tempMatrix[i].size()-1)){
                        if (tempMatrix[i-1][j+1] != -1){
                            tempMatrix[i-1][j+1] = tempMatrix[i-1][j+1]+1;
                        }
                    }
                }
                if (j>0){   // square W of mine
                    if (tempMatrix[i][j-1] != -1){
                        tempMatrix[i][j-1] = tempMatrix[i][j-1]+1;
                    }
                }
                if (j<(tempMatrix[i].size()-1)){    // square E of mine
                    if (tempMatrix[i][j+1] != -1){
                        tempMatrix[i][j+1] = tempMatrix[i][j+1]+1;
                    }
                }
                if (i<(tempMatrix.size()-1)){   // top 3 squares below mine (SW, S, SE)
                    if (tempMatrix[i+1][j] != -1){
                        tempMatrix[i+1][j] = tempMatrix[i+1][j]+1;
                    }
                    if (j>0){
                        if (tempMatrix[i+1][j-1] != -1){
                            tempMatrix[i+1][j-1] = tempMatrix[i+1][j-1]+1;
                        }
                    }
                    if (j<(tempMatrix[i].size()-1)){
                        if (tempMatrix[i+1][j+1] != -1){
                            tempMatrix[i+1][j+1] = tempMatrix[i+1][j+1]+1;
                        }
                    }
                }

            }
        }
    }
    
    mineAmtMatrix = tempMatrix;


    if (markedCellsMatrix.size() == 0){     // initializing marked cells matrix
        for (i=0; i<mineAmtMatrix.size(); i++){
            for (j=0; j<mineAmtMatrix[i].size(); j++){
                tempVectorBool.push_back(false);
            }
            tempMatrixBool.push_back(tempVectorBool);
            tempVectorBool.clear();
        }
        markedCellsMatrix = tempMatrixBool;
    }

    tempVectorBool.clear();
    tempMatrixBool.clear();

    if (revealedCellMatrix.size() == 0){     // initializing revealed cells matrix
        for (i=0; i<mineAmtMatrix.size(); i++){
            for (j=0; j<mineAmtMatrix[i].size(); j++){
                tempVectorBool.push_back(false);
            }
            tempMatrixBool.push_back(tempVectorBool);
            tempVectorBool.clear();
        }
        revealedCellMatrix = tempMatrixBool;
    }

    // Code to print the matrix (debug)
    /*for (i=0; i<tempMatrix.size(); i++){
        for (j=0; j<tempMatrix[i].size(); j++){
            cout << tempMatrix[i][j] << "\t";
        }
        cout << endl;
    }*/
}

bool gameMatrix::checkMarking(sf::Vector2i mouseIndexPos){
    int long unsigned i=0, j=0;
    /*STRAT:
        Have a matrix that contains bools (true if pos marked by the user, false otherwise)
        If a mine was marked by the user, change its tag in the mineAmtMatrix from -1 to -2
        Check if there are any more mines with -1 tag, if not, user wins
    */
    // [y][x]
    if (markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x] == true){
        markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x] = false;
    }else{
        //if (mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] )
        markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x] = true;
    }

    if (mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] == -1 && markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x]){
        mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] = -2;
    }else{
        if (mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] == -2 && !markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x]){
            mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] = -1;
        }
    }

    // Debug code to print matrices
    /*for (i=0; i<mineAmtMatrix.size(); i++){
        for (j=0; j<mineAmtMatrix[i].size(); j++){
            cout << mineAmtMatrix[i][j] << "\t";
        }
        cout << endl;
    }

    for (i=0; i<markedCellsMatrix.size(); i++){
        for (j=0; j<markedCellsMatrix[i].size(); j++){
            cout << markedCellsMatrix[i][j] << "\t";
        }
        cout << endl;
    }
    cout << endl;*/

    return markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x];
}

int gameMatrix::checkAction(sf::Vector2i mouseIndexPos){
    /*
    POSSIBLE RESULTS:
        Unmarked mine clicked: 0
        Marked square clicked: 1
        Empty (0) space clicked: 2
        Numbered but not empty space (>0): 3
    */
    int result=0;

        if (mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] == -1){
            // Mine clicked
            result = 0;
        }else{
            if (markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x] == true){
                if (mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] == -2){
                    // Marked mine was clicked
                    mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] = -1;
                }
                // Marked square was clicked
                result = 1;

            }else{
                if (mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] == 0){
                    result = 2;

                }else{
                    if (mineAmtMatrix[mouseIndexPos.y][mouseIndexPos.x] > 0){
                        result = 3;
                    }
                }
            }
        }

        if (result >= 1 && result <= 3){
            markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x] = false;
        }


    return result;

}

void gameMatrix::checkRevealed(sf::Vector2i mouseIndexPos){
    bool result=false;
    int long unsigned i=0, j=0, k=0;

    /*DO HERE:
        Modify revealed matrix to show true when a cell was revealed
        Check if adjacent cells should be revealed or not
    */

    if (markedCellsMatrix[mouseIndexPos.y][mouseIndexPos.x] == false){
        revealedCellMatrix[mouseIndexPos.y][mouseIndexPos.x] = true;
    }

    // Loop that increments the number on each matrix square around a mine
    for (i=0; i<revealedCellMatrix.size(); i++){
        for (j=0; j<revealedCellMatrix[i].size(); j++){

            if (mineAmtMatrix[i][j] == 0){
                revealedCellMatrix[i][j] = true;

                if (i>0){   // top 3 squares above mine (NW, N, NE)
                    revealedCellMatrix[i-1][j] = true;
                    if (j>0){
                        revealedCellMatrix[i-1][j-1] = true;
                    }
                    if (j<(revealedCellMatrix[i].size()-1)){
                        revealedCellMatrix[i-1][j+1] = true;
                    }
                }
                if (j>0){   // square W of mine
                    revealedCellMatrix[i][j-1] = true;
                }
                if (j<(revealedCellMatrix[i].size()-1)){    // square E of mine
                    revealedCellMatrix[i][j+1] = true;
                }
                if (i<(revealedCellMatrix.size()-1)){   // top 3 squares below mine (SW, S, SE)
                    revealedCellMatrix[i+1][j] = true;
                    if (j>0){
                        revealedCellMatrix[i+1][j-1] = true;
                    }
                    if (j<(revealedCellMatrix[i].size()-1)){
                        revealedCellMatrix[i+1][j+1] = true;
                    }
                }

            }
        }
    }
    
    // DEBUG: Prints revealed matrix every time the user right-clicks
    /*for (i=0; i<revealedCellMatrix.size(); i++){
        for (j=0; j<revealedCellMatrix[i].size(); j++){
            cout << revealedCellMatrix[i][j] << "\t";
        }
        cout << endl;
    }*/

}


void showMessage(string message, int duration_secs, int unsigned charSize_px=0, sf::Color msgColor=ENDGAME_TEXT_COLOR){
    int count=0;
    bool moveUp=true;

    sf::RenderWindow msgWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().height/1.2, sf::VideoMode::getDesktopMode().height/2), "");
    // Window size depends on maximum height for both dimensions, so that the window is aprox. the same size on most displays
    msgWindow.setFramerateLimit(MAX_FRAMERATE);

    if (charSize_px == 0){       // This means 'auto-size' option
        charSize_px = sf::VideoMode::getDesktopMode().height/8;
    }
    sf::Font pixelFont;
    if (!pixelFont.loadFromFile(PIXEL_FONT_PATH)){
        cout << "Pixel font not found" << endl;
        msgWindow.close();
    }

    sf::Text msgText(message, pixelFont, charSize_px);
    msgText.setFillColor(msgColor);

    sf::Vector2f centerPos(msgText.getGlobalBounds().width / 2.f, msgText.getGlobalBounds().height / 2.f);
    sf::Vector2f rounded = roundPosVector(centerPos);     // done so text doesnt get blurry if a decimal of a pixel was set as origin
    msgText.setOrigin(rounded);

    msgText.setPosition(sf::Vector2f(msgWindow.getSize().x / 2u, msgWindow.getSize().y / 4u));
    
    sf::Texture beachTexture;
    if(!beachTexture.loadFromFile(END_BACKGROUND_TEXTURE_PATH)){
        cout << "Background texture not found" << endl;
        msgWindow.close();
    }
    sf::Sprite beachSprite(beachTexture);       // create wave sprite as well
    beachSprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), END_BG_IMG_SIZE));
    beachSprite.setScale(sf::Vector2f(msgWindow.getSize().x / END_BG_IMG_SIZE.x, msgWindow.getSize().y / END_BG_IMG_SIZE.y));
    beachSprite.setPosition(sf::Vector2f(0, msgWindow.getSize().y - (END_BG_IMG_SIZE.y*beachSprite.getScale().y)/1.5));

    sf::Sprite waveSprite(beachTexture);       // create wave sprite as well
    waveSprite.setTextureRect(sf::IntRect(sf::Vector2i(END_BG_IMG_SIZE.x, 0), END_BG_IMG_SIZE));
    waveSprite.setScale(sf::Vector2f(msgWindow.getSize().x / END_BG_IMG_SIZE.x, msgWindow.getSize().y / END_BG_IMG_SIZE.y));
    waveSprite.setPosition(sf::Vector2f(0, msgWindow.getSize().y - (END_BG_IMG_SIZE.y*waveSprite.getScale().y)));

    while (msgWindow.isOpen()){
        sf::Event event;

        while (msgWindow.pollEvent(event)){
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))){
                msgWindow.close();
            }
        }

        if (count == MAX_FRAMERATE*duration_secs){      // framerate (frames / second) * n  ==  n seconds, because count will increase at most by MAX_FRAMERATE every second
            msgWindow.close();
        }
        
        if (!moveUp){
            waveSprite.setPosition(sf::Vector2f(waveSprite.getPosition().x, waveSprite.getPosition().y+1));
        }else{
            waveSprite.setPosition(sf::Vector2f(waveSprite.getPosition().x, waveSprite.getPosition().y-1));
        }

        if ((count%MAX_FRAMERATE == 0) && !moveUp && (count/MAX_FRAMERATE)%2 == 0){        // On odd seconds do this
            moveUp = true;
        }else{
            if (count%MAX_FRAMERATE == 0 && moveUp){                                      // On even ones do this
                moveUp = false;
            }
        }

        msgWindow.clear(END_BG_COLOR);
        msgWindow.draw(beachSprite);
        msgWindow.draw(waveSprite);
        msgWindow.draw(msgText);

        msgWindow.display();
        count++;

    }

}


bool mineGame(){
    int long unsigned j=0, i=0;
    unsigned int count=0, markedCount=0;

    sf::RenderWindow window(sf::VideoMode(sf::VideoMode::getDesktopMode().height/1.2, sf::VideoMode::getDesktopMode().height/1.2), "");
    // Window size depends on maximum height for both dimensions, so that the window is aprox. the same size on most displays
    window.setFramerateLimit(MAX_FRAMERATE);

    gameMatrix matx(window, MATRIX_SIZE);
    vector<vector<sf::Vertex>> linesVector = matx.getLinesVector();
    sf::Vertex lineToDraw[2];

    sf::Vector2i mousePos, matxRectIndexPos;

    sf::RectangleShape hoverRect(sf::Vector2f((matx.getDivWidth().x-1), (matx.getDivWidth().y-1)));
    hoverRect.setFillColor(HOVER_COLOR);

    vector<sf::RectangleShape> markedSquares;
    vector<sf::RectangleShape *> markedSquaresToDraw;

    vector<sf::Text> squareLabels;
    vector<sf::Text *> squareLabelsToDraw;

    sf::Font pixelFont;
    if (!pixelFont.loadFromFile(PIXEL_FONT_PATH)){
        cout << "Error loading font" << endl;
        window.close();
    }

    int clickAction=0, squareIndex=-1;
    bool checkMarkedSquare=false, revealedSquare=false, gameWon=true, closeGame=false, returnValue=false;
    
    while (window.isOpen()){
        sf::Event event;
        mousePos = sf::Mouse::getPosition(window);
        matxRectIndexPos = matx.getMousePosIndex(mousePos);

        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))){
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left) && !closeGame){
                if (matx.getMineMatrix().size() == 0){
                    matx.generateMineMatrix(matxRectIndexPos);
                }
                clickAction = matx.checkAction(matxRectIndexPos);
                switch (clickAction)
                {
                case 0:     // Unmarked mine clicked
                    // cout << "Unmarked mine clicked" << endl;
                    window.close();
                    returnValue = false;
                    break;
                case 1:     // Marked square (mine or not) clicked
                    // cout << "Marked square clicked" << endl;
                    for (i=0; i<markedSquares.size(); i++){
                        if (markedSquares[i].getPosition().x == (matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y].x) && markedSquares[i].getPosition().y == (matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y].y+1)){
                            squareIndex = i;
                        }
                    }
                    if (squareIndex != -1){     // squareIndex will be -1 if marked square was not found (shouldn't happen)
                        markedSquares.erase(markedSquares.begin()+squareIndex);
                        markedSquaresToDraw.erase(markedSquaresToDraw.begin()+squareIndex);
                    }
                    squareIndex = -1;   // resets squareIndex in case it is needed again later
                    break;
                case 3:     // Numbered space clicked
                    // cout << "Numbered space clicked" << endl;

                    squareLabels.push_back(sf::Text(to_string(matx.getMineMatrix()[matxRectIndexPos.y][matxRectIndexPos.x]), pixelFont, matx.getDivWidth().x*0.9));
                    squareLabelsToDraw.push_back(&(squareLabels[squareLabels.size()-1]));

                    squareLabelsToDraw[squareLabels.size()-1]->setFillColor(NORMAL_TEXT_COLOR);
                    squareLabelsToDraw[squareLabels.size()-1]->setPosition(sf::Vector2f((matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y].x+(matx.getDivWidth().x/4)), (matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y].y-(matx.getDivWidth().y/8))));

                    markedSquares.push_back(sf::RectangleShape(sf::Vector2f((matx.getDivWidth().x-1), (matx.getDivWidth().y-1))));
                    markedSquaresToDraw.push_back(&(markedSquares[markedSquares.size()-1]));

                    markedSquaresToDraw[markedSquaresToDraw.size()-1]->setFillColor(CLICKED_SPACE_COLOR);
                    markedSquaresToDraw[markedSquaresToDraw.size()-1]->setPosition((matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y]));
                    markedSquaresToDraw[markedSquaresToDraw.size()-1]->setPosition(sf::Vector2f(markedSquaresToDraw[markedSquaresToDraw.size()-1]->getPosition().x, markedSquaresToDraw[markedSquaresToDraw.size()-1]->getPosition().y+1));
                    
                    break;

                default:    // Empty space clicked
                    // cout << "Empty space clicked" << endl;

                    markedSquares.push_back(sf::RectangleShape(sf::Vector2f((matx.getDivWidth().x-1), (matx.getDivWidth().y-1))));
                    markedSquaresToDraw.push_back(&(markedSquares[markedSquares.size()-1]));

                    markedSquaresToDraw[markedSquaresToDraw.size()-1]->setFillColor(CLICKED_SPACE_COLOR);
                    markedSquaresToDraw[markedSquaresToDraw.size()-1]->setPosition((matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y]));
                    markedSquaresToDraw[markedSquaresToDraw.size()-1]->setPosition(sf::Vector2f(markedSquaresToDraw[markedSquaresToDraw.size()-1]->getPosition().x, markedSquaresToDraw[markedSquaresToDraw.size()-1]->getPosition().y+1));
                    
                    matx.checkRevealed(matxRectIndexPos);

                    for (i=0; i<matx.getRevMatrix().size(); i++){
                        for (j=0; j<matx.getRevMatrix()[i].size(); j++){
                            if (matx.getRevMatrix()[i][j]){
                                markedSquares.push_back(sf::RectangleShape(sf::Vector2f((matx.getDivWidth().x-1), (matx.getDivWidth().y-1))));
                                markedSquaresToDraw.push_back(&(markedSquares[markedSquares.size()-1]));

                                markedSquaresToDraw[markedSquaresToDraw.size()-1]->setFillColor(CLICKED_SPACE_COLOR);
                                markedSquaresToDraw[markedSquaresToDraw.size()-1]->setPosition((matx.getGridMatrix()[j][i]));
                                markedSquaresToDraw[markedSquaresToDraw.size()-1]->setPosition(sf::Vector2f(markedSquaresToDraw[markedSquaresToDraw.size()-1]->getPosition().x, markedSquaresToDraw[markedSquaresToDraw.size()-1]->getPosition().y+1));

                                if (matx.getMineMatrix()[i][j] > 0){
                                    squareLabels.push_back(sf::Text(to_string(matx.getMineMatrix()[i][j]), pixelFont, matx.getDivWidth().x*0.9));
                                    squareLabelsToDraw.push_back(&(squareLabels[squareLabels.size()-1]));

                                    squareLabelsToDraw[squareLabels.size()-1]->setFillColor(NORMAL_TEXT_COLOR);
                                    squareLabelsToDraw[squareLabels.size()-1]->setPosition(sf::Vector2f((matx.getGridMatrix()[j][i].x+(matx.getDivWidth().x/4)), (matx.getGridMatrix()[j][i].y-(matx.getDivWidth().y/8))));

                                }
                                
                            }
                        }
                    }
                    break;
                }
                
                // DEBUG: Prints mine matrix every time the user clicks
                /*for (i=0; i<matx.getMineMatrix().size(); i++){
                    for (j=0; j<matx.getMineMatrix()[i].size(); j++){
                        cout << matx.getMineMatrix()[i][j] << "\t";
                    }
                    cout << endl;
                }
                cout << endl;*/

            }
            if (matx.getMineMatrix().size() != 0){
                if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Right) && !closeGame){
                    checkMarkedSquare = matx.checkMarking(matxRectIndexPos);

                    // Search for any -1 s, if none, the user has won
                    for (i=0; i<matx.getMineMatrix().size(); i++){
                        for (j=0; j<matx.getMineMatrix()[i].size(); j++){
                            if(matx.getMineMatrix()[i][j] == -1){
                                gameWon = false;
                            }
                        }
                    }
                    if (gameWon){       // If gameWon is still true, then no -1's were found (all mines are marked)
                        // Game was won, should move to the end screen
                        closeGame = true;
                    }else{
                        gameWon = true;     // Needs to be true for next iteration
                    }

                    if (checkMarkedSquare){
                        markedCount++;
                        markedSquares.push_back(sf::RectangleShape(sf::Vector2f((matx.getDivWidth().x-1), (matx.getDivWidth().y-1))));
                        markedSquaresToDraw.push_back(&(markedSquares[markedSquares.size()-1]));

                        markedSquaresToDraw[markedSquaresToDraw.size()-1]->setFillColor(MARKED_COLOR);
                        markedSquaresToDraw[markedSquaresToDraw.size()-1]->setPosition((matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y]));
                        markedSquaresToDraw[markedSquaresToDraw.size()-1]->setPosition(sf::Vector2f(markedSquaresToDraw[markedSquaresToDraw.size()-1]->getPosition().x, markedSquaresToDraw[markedSquaresToDraw.size()-1]->getPosition().y+1));
                    }else{
                        for (i=0; i<markedSquares.size(); i++){
                            if (markedSquares[i].getPosition().x == (matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y].x) && markedSquares[i].getPosition().y == (matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y].y+1)){
                                squareIndex = i;
                            }
                        }
                        if (squareIndex != -1){     // squareIndex will be -1 if marked square was not found (shouldn't happen)
                            markedCount--;
                            markedSquares.erase(markedSquares.begin()+squareIndex);
                            markedSquaresToDraw.erase(markedSquaresToDraw.begin()+squareIndex);
                        }
                        squareIndex = -1;
                    }
                }
            }
        }
        
        hoverRect.setPosition((matx.getGridMatrix()[matxRectIndexPos.x][matxRectIndexPos.y]));
        hoverRect.setPosition(sf::Vector2f(hoverRect.getPosition().x, hoverRect.getPosition().y+1));

        window.clear(BG_COLOR);

        for (i=0; i<linesVector.size(); i++){
            lineToDraw[0] = linesVector[i][0];
            lineToDraw[1] = linesVector[i][1];
            window.draw(lineToDraw, 2, sf::Lines);
        }

        if (!(mousePos.x > window.getSize().x) && !(mousePos.y > window.getSize().y)){
            // if mouse is not outside window, then draw the hover rectangle
            window.draw(hoverRect);
        }

        if (markedSquares.size() != 0){
            for (i=0; i<markedSquares.size(); i++){
                window.draw(((markedSquares[i])));
            }
        }

        if (squareLabels.size() != 0){
            for (i=0; i<squareLabels.size(); i++){
                window.draw(squareLabels[i]);
            }
        }

        window.display();

        if (count == UINT32_MAX){
            count = 0;
        }

        if (closeGame){
            count++;
            window.setTitle("Congratulations, you won the game!");

            if (count == MAX_FRAMERATE*1){      // framerate (frames / second) * 1  ==  1 second
                window.close();
                returnValue = true;
            }
        }else{
            if (matx.numberOfMines == -1){
                window.setTitle("Playing on a " + to_string(matx.getGridMatrix().size()) + "x" + to_string(matx.getGridMatrix().size()) + " mine field");
            }else{
                window.setTitle("Playing on a " + to_string(matx.getGridMatrix().size()) + "x" + to_string(matx.getGridMatrix().size()) + " mine field,  " + to_string(markedCount) + " / " + to_string(matx.numberOfMines) + " marks placed");
            }
        }
    }
    return returnValue;
}


int selectDifficulty(){
    sf::RenderWindow diffWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().height/1.2, sf::VideoMode::getDesktopMode().height/2), "Select difficulty");
    diffWindow.setFramerateLimit(MAX_FRAMERATE);

    sf::Font pixelFont;
    if (!pixelFont.loadFromFile(PIXEL_FONT_PATH)){
        cout << "Pixel font not found" << endl;
        diffWindow.close();
    }

    vector<sf::Text> textDiffVector;      // easyText, midText, hardText, titleText

    textDiffVector.push_back(sf::Text("Select difficulty", pixelFont, 45));     // Title
    textDiffVector.push_back(sf::Text("- Easy", pixelFont, 30));
    textDiffVector.push_back(sf::Text("- Medium", pixelFont, 30));
    textDiffVector.push_back(sf::Text("- Hard", pixelFont, 30));


    while (diffWindow.isOpen()){

        diffWindow.clear(sf::Color::Black);
        diffWindow.draw(textDiffVector[0]);
        diffWindow.display();
    }

    return 0;

}

int main(){
    bool userWin;
    
    selectDifficulty();

    /*userWin = mineGame();
    
    if (userWin){
        showMessage("GAME WON!", 5);
    }else{
        showMessage("YOU LOST!", 5);
    }*/


    return 0;
}