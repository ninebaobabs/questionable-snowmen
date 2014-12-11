
#include "game.h"




internal COLOR
MakeColor(int r, int g, int b)
{
    // i think this got messed up when switching to double buffering
    return (COLOR)( (r<<16) | (b<<8) | (g) );  // totally wrong
    // return (COLOR)( (b<<16) | (g<<8) | (r) ); // this is actuall correct (is it?:?)
    //  xx rr gg bb
    //  xx bb gg rr
}


internal void
RenderBufferOnBuffer(game_buffer *SrceBuffer, game_buffer *DestBuffer)
{
    for (int y = 0; y < DestBuffer->Height; y++)
    {
        for (int x = 0; x < DestBuffer->Width; x++)
        {
            int pixelOffset = y*DestBuffer->Width + x;
            *(((uint32_t*)DestBuffer->Memory) + pixelOffset) = 
            *(((uint32_t*)SrceBuffer->Memory) + pixelOffset);
        }
    }
}


internal void
FillBuffer(game_buffer *Buffer, COLOR Color)
{
    for (int y = 0; y < Buffer->Height; y++)
    {
        for (int x = 0; x < Buffer->Width; x++)
        {
            int pixelOffset = y*Buffer->Width + x;
            *(((uint32_t*)Buffer->Memory) + pixelOffset) = Color;
        }
    }
}

internal void
FillRectInBuffer(rect Box, game_buffer *Buffer, COLOR Color)
{
    for (int y = (int)Box.y; y < (int)Box.y+(int)Box.h; y++)
    {
        for (int x = (int)Box.x; x < (int)Box.x+(int)Box.w; x++)
        {
            if (x >= 0 && y >= 0 && x < Buffer->Width && y < Buffer->Height)
            {
                int pixelOffset = y*Buffer->Width + x;
                *(((uint32_t*)Buffer->Memory) + pixelOffset) = Color;
            }
        }
    }
}


internal void
BakeIntoBG (rect tang, game_buffer *BGBuffer) 
{
    FillRectInBuffer(tang, BGBuffer, DeadPieceColor);
}


internal rect
AddRects(rect r1, rect r2)
{
    return {r1.x+r2.x, r1.y+r2.y, r1.w+r2.w, r1.h+r2.h};
}


internal void
DrawSnowMound(float x, float y, game_buffer *Buffer)
{
    rect mound = {x, y, 36, 16};
    FillRectInBuffer(mound, Buffer, SnowMound);

    float shrink = 7;
    rect mound2 = {x-shrink, y+shrink, 36+shrink*2, 16-shrink};
    FillRectInBuffer(mound2, Buffer, SnowMound);
}


internal void
RenderBG(game_buffer *Buffer)
{
    FillBuffer(Buffer, Ground);
    DrawSnowMound(600, 500, Buffer);
    DrawSnowMound(200, 125, Buffer);
    DrawSnowMound(185, 400, Buffer);
    DrawSnowMound(861, 300, Buffer);
    DrawSnowMound(700, 20, Buffer);
    DrawSnowMound(1100, 600, Buffer);
    DrawSnowMound(900, 700, Buffer);
    DrawSnowMound(30, 550, Buffer);
}


internal rect
AutoRectOnTop(rect baseRect, int xShift, int yShift, int xShrink, int yShrink)
{
    int oldCenterX = baseRect.x + baseRect.w/2;
    int oldCenterY = baseRect.y + baseRect.h/2;

    int newCenterX = oldCenterX + xShift;
    int newCenterY = oldCenterY + yShift;

    int newW = baseRect.w - xShrink;
    int newH = baseRect.h - yShrink;

    newCenterY -= newH;  // plop on top

    int newX = newCenterX - newW/2;
    int newY = newCenterY - newH/2;

    return { newX, newY, newW, newH };
}



// should be CalcSnowManShadowRect ?
// shouoldn't this all be float?
internal rect CalcSnowManRect(snow_person man) 
{
    int minX = 1000000;
    int maxX = -100000;
    int minY = 1000000;
    int maxY = -100000;

    int totalX = 0;
    int totalY = 0;
    int pieceCount = 0;

    for (int j = 0; j < 4; j++)
    {
        if (!man.pieces[j].dead)
        {
            int px = man.pieces[j].pos.x;
            int py = man.pieces[j].pos.y;
            int pr = man.pieces[j].pos.w + px;
            int pb = man.pieces[j].pos.h + py;

            if (px < minX) minX = px;
            if (pr > maxX) maxX = pr;
            if (py < minY) minY = py;
            if (pb > maxY) maxY = pb;

            totalX += px;
            totalY += py;

            pieceCount++;
        }
    }

    int averageX = (maxX-minX)/2 + minX; //totalX/pieceCount;
    int averageY = (maxY-minY)/2 + minY; //totalY/pieceCount;
    int width = maxX-minX+12;
    int height = (maxY-minY)/7;

    return { averageX-width/2, maxY - height/2 + man.height, width, height };
}



internal snow_person MakeSnowPerson(int x, int y)
{
    snow_person newPerson = { };

    newPerson.base.pos = { x, y, 36, 28 };
    newPerson.mid.pos = AutoRectOnTop(newPerson.base.pos, 0, 0, 6, 3);
    newPerson.head.pos = AutoRectOnTop(newPerson.mid.pos, 0, 0, 6, 3);
    newPerson.hat.pos = AutoRectOnTop(newPerson.head.pos, 0, -3, 3, 3);
    newPerson.shadowPos = { x-4, y + 22, 42, 16 };

    newPerson.base.dead = false;
    newPerson.mid.dead = false;
    newPerson.head.dead = false;
    newPerson.hat.dead = false;

    newPerson.base.damage = 0;
    newPerson.mid.damage = 0;
    newPerson.base.damage = 0;
    newPerson.hat.damage = 0;

    newPerson.moveTarget = {};
    newPerson.allDead = false;

    for (int i = 0; i < 4; i++)
    {
        newPerson.delays[i] = 0;
        newPerson.tx[i] = 0;
        newPerson.ty[i] = 0;
    }

    float RandomAngle = ((float)RandomIntLessThan(360)) / 3.14159f/180.0f;
    float RandomRadius = SnowmanStrikeRadius;//RandomIntLessThan(SnowmanStrikeRadius);
    newPerson.targetOffsetX = cosf(RandomAngle) * RandomRadius;
    newPerson.targetOffsetY = sinf(RandomAngle) * RandomRadius;

    int maxSpeedPercet = 100 + SnowmanSpeedPercentRange/2;
    int minSpeedPercet = 100 - SnowmanSpeedPercentRange/2;
    newPerson.randomSpeedFactor = ( (float)RandomIntLessThan(SnowmanSpeedPercentRange)-
                                   SnowmanSpeedPercentRange/2 + 100 ) 
                                    /100.0f;

    return newPerson;
}


internal void
DrawRectAsTopHat(rect hat, game_buffer *Buffer, COLOR col)
{
    // rim is always 6 longer than width and 1/4 of height
    rect rim = AutoRectOnTop(hat, 0, hat.h-2, -8, hat.h-6);

    FillRectInBuffer(hat, Buffer, col);
    FillRectInBuffer(rim, Buffer, col);
}


internal void
DrawSnowMan(snow_person person, game_buffer *Buffer)
{
    // for (int i = 0; i < 3; i++)
    // {
    //  if (!person.pieces[i].dead)
    //  {
    //      FillRectInBuffer(person.pieces[i].pos, Buffer, SnowmanBody);
    //  }
    // }


    if (!person.base.dead)
        FillRectInBuffer(person.base.pos, Buffer, SnowmanBody);
    if (!person.mid.dead)
        FillRectInBuffer(person.mid.pos, Buffer, SnowmanBody);
    if (!person.head.dead)
        FillRectInBuffer(person.head.pos, Buffer, SnowmanBody);

    if (!person.hat.dead)
    {
        DrawRectAsTopHat(person.hat.pos, Buffer, TopHat);
    }
}



internal void
DrawDeadPlayer(rect BasePos, game_buffer *Buffer)
{
    FillRectInBuffer(BasePos, Buffer, PlayerBodyDead);

    rect gloveL = {-16, 8, -16, -10};
    FillRectInBuffer(AddRects(BasePos, gloveL), Buffer, PlayerMittsDead);
    rect gloveR = {32, 8, -16, -10};
    FillRectInBuffer(AddRects(BasePos, gloveR), Buffer, PlayerMittsDead);

    // rect HatMainOffsets = {-24, 0, -6, -6};
    // FillRectInBuffer(AddRects(BasePos, HatMainOffsets), Buffer, PlayerHat);

    // rect HatRimOffsets = {-6, -6, 12, -16};
    // FillRectInBuffer(AddRects(BasePos, HatRimOffsets), Buffer, PlayerHat);

//  DrawTopHat(BasePos, Buffer);
}

internal void
DrawPlayer(rect BasePos, game_buffer *Buffer)
{
    FillRectInBuffer(BasePos, Buffer, PlayerBody);

    rect gloveL = {-16, 0, -16, -10};
    FillRectInBuffer(AddRects(BasePos, gloveL), Buffer, PlayerMitts);
    rect gloveR = {32, 0, -16, -10};
    FillRectInBuffer(AddRects(BasePos, gloveR), Buffer, PlayerMitts);

    // rect HatMainOffsets = {-24, 0, -6, -6};
    // FillRectInBuffer(AddRects(BasePos, HatMainOffsets), Buffer, PlayerHat);

    // rect HatRimOffsets = {-6, -6, 12, -16};
    // FillRectInBuffer(AddRects(BasePos, HatRimOffsets), Buffer, PlayerHat);

//  DrawTopHat(BasePos, Buffer);
}


internal bool
RectsCollide(rect r1, rect r2)
{
    int r1r = r1.x+r1.w;
    int r1b = r1.y+r1.h;
    int r2r = r2.x+r2.w;
    int r2b = r2.y+r2.h;

    if (r1r < r2.x || r1b < r2.y || r2r < r1.x || r2b < r1.y)
        return false;
    else
        return true;
}

internal bool
MiniRectCollidesWithFullRect(rect r1, rect r2)
{
    int shrink = 5;  // size is 14 right now

    int r1r = r1.x+r1.w;
    int r1b = r1.y+r1.h;
    int r2r = r2.x+r2.w;
    int r2b = r2.y+r2.h;

    if (r1r-shrink < r2.x        || 
        r1b-shrink < r2.y        || 
        r2r        < r1.x+shrink || 
        r2b        < r1.y+shrink )
        return false;
    else
        return true;
}

internal bool
RectCenterHits(rect r1, rect r2)
{
    int r1cx = r1.x + r1.w/2;
    int r1cy = r1.y + r1.h/2;
    int r2r = r2.x+r2.w;
    int r2b = r2.y+r2.h;

    if (r1cx > r2.x && 
        r1cy > r2.y && 
        r1cx < r2r && 
        r1cy < r2b)
        return true;
    else
        return false;
}


// 0 = miss, 1-4 = hit that body part
internal int
SnowballHitsSnowman(snow_ball ball, snow_person man)
{
    for (int i = 0; i < 4; i++)
    {
        if (!man.pieces[i].dead)
        {
            // if (RectsCollide(ball.colPos, man.pieces[i].pos))
            // if (RectCenterHits(ball.colPos, man.pieces[i].pos))
            if (MiniRectCollidesWithFullRect(ball.colPos, man.pieces[i].pos))
            {
                return (i+1);
            }
        }
    }
    return 0;
}


internal int
SnowManBasePosY(snow_person man)
{
    for (int i = 0; i < 4; i++)
    {
        if (!man.pieces[i].dead)
        {
            return man.pieces[i].pos.y;
        }
    }
    // hit something totally dead?
    return 0;
}


// internal void
// ResolveQueuedMovements(snow_person *man, float dt)
// {
//     // start at mid, base never follows
//     for (int j = 1; j < 4; j++)
//     {
//         if (!man->pieces[j].dead)
//         {
//             if (man->pieces[j].moveTowardsParentDelay > 0)
//             {
//                 man->pieces[j].moveTowardsParentDelay -= dt;
//             }
//             if (man->pieces[j].moveTowardsParentDelay <= 0)
//             {
//                 man->pieces[j].moveTowardsParentDelay = 0;

//                 if (j == 0

//             }
//         }
//     }
// }

global_variable float SnowManAnimSpeed = 0.4f;

internal void
StepAnimateMoveEntireSnowMan(snow_person *man, float dt)
{
    // start at mid, base never follows
    for (int j = 1; j < 4; j++)
    {
        if (!man->pieces[j].dead)
        {
            if (man->delays[j] > 0)
            {
                man->delays[j] -= dt;
            }
            if (man->delays[j] <= 0)
            {
                if ((man->tx[j] < -.01 || man->tx[j] > .01)
                 || (man->ty[j] < -.01 || man->ty[j] > .01))
                {
                    man->delays[j] = 0;

                    float tx = man->tx[j]; //man->pieces[j-1].pos.x;
                    float ty = man->ty[j]; //man->pieces[j-1].pos.y;
                    float ax = man->pieces[j].pos.x;
                    float ay = man->pieces[j].pos.y;

                    float dist = sqrt(((tx-ax)*(tx-ax) + (ty-ay)*(ty-ay)));

                    float ux = (tx-ax) / dist;
                    float uy = (ty-ay) / dist;

                    man->pieces[j].pos.x += SnowManAnimSpeed * ux * dt;
                    man->pieces[j].pos.y += SnowManAnimSpeed * uy * dt;
                    man->tx[j] -= SnowManAnimSpeed * ux * dt;
                    man->ty[j] -= SnowManAnimSpeed * uy * dt;
                }
                else
                {
                    man->tx[j] = 0;
                    man->ty[j] = 0;
                }
            }
        }
    }
}


// internal *rect
// ActualBase(snow_person *man)
// {
//     for (int j = 0; j < 4; j++)
//     {
//         if (!man->pieces[j].dead)
//         {
//             man->pieces[j].pos.x += dx;
//             man->pieces[j].pos.y += dy;
//         }
//     }
// }

internal float
GetSnowManBaseCenterPosX(snow_person man)
{
    for (int j = 0; j < 4; j++)
    {
        if (!man.pieces[j].dead)
        {
            return man.pieces[j].pos.x + man.pieces[j].pos.w/2;
        }
    }
    return -100;
}
internal float
GetSnowManBaseCenterPosY(snow_person man)
{
    for (int j = 0; j < 4; j++)
    {
        if (!man.pieces[j].dead)
        {
            return man.pieces[j].pos.y + man.pieces[j].pos.h/2;
        }
    }
    return -100;
}


global_variable float SnowManAnimDelay = 0.05f;

internal void 
SetAnimateMoveEntireSnowMan(snow_person *man, int dx, int dy)
{
    for (int j = 0; j < 4; j++)
    {
        if (!man->pieces[j].dead)
        {
            man->pieces[j].pos.x += dx;
            man->pieces[j].pos.y += dy;

            // queue up movement to follow on the rest..
            for (int i = j+1; i < 4; i++)
            {
                //man->pieces[pieceIndex].queuedMove = { dx, dy, 0, 0 };
                man->delays[i] = SnowManAnimDelay * (i-j);

            man->pieces[i].pos.x += dx;
            man->pieces[i].pos.y += dy;

                // man->tx[i] += dx;
                // man->ty[i] += dy;

            }

            return;
        }
    }
}


internal void
ForceMoveEntireSnowMan(snow_person *man, float dx, float dy)
{
    for (int j = 0; j < 4; j++)
    {
        if (!man->pieces[j].dead)
        {
            man->pieces[j].pos.x += dx;
            man->pieces[j].pos.y += dy;
        }
    }
}



internal void
SnowManApplyGrav(snow_person *man, float dt)
{


    float manGrav = 0.20f;
    if (man->height > 0.0f)
    {
        man->height -= manGrav * dt;
        ForceMoveEntireSnowMan(man, 0, manGrav * dt);
        // SetAnimateMoveEntireSnowMan(man, 0, manGrav * dt);

        if (man->height < 0) man->height = 0;
    }
}




internal void
NewSnowManApplyGrav(snow_person *man, float dt)
{

    float manGrav = 0.20f;
    if (!man->landed)
    {
        if (man->jumpUpLeft > 0.0f)  // is jumping up
        {
            man->jumpUpLeft -= dt;
            manGrav *= -1;

            if (man->jumpUpLeft <= 0.0f) man->jumpUpLeft = 0.0f;
        }
        else
        {
            manGrav *= 1;
        }


        if (man->height > 0.0f || manGrav<0)
        {
            man->height -= manGrav * dt;
            ForceMoveEntireSnowMan(man, 0, manGrav * dt);
        }

        if (man->height <= 0.0f)
        {
            man->landed = true;
            man->timeTillNextJump = SnowmanJumpDelay;

            if (man->height < 0) man->height = 0; // possible bug?
        }
    }
    else  // on ground
    {
        man->timeTillNextJump -=dt;
        if (man->timeTillNextJump <= 0.0f)
        {
            man->landed = false;
            man->jumpUpLeft = SnowmanJumpLength;
        }
    }

}



internal void
ResetDeadSnowMan(snow_person *man, game_buffer *Screen) {
    int side = RandomIntLessThan(4);
    if (side == 0) *man = MakeSnowPerson(RandomIntLessThan(Screen->Width), -100);
    if (side == 2) *man = MakeSnowPerson(RandomIntLessThan(Screen->Width), Screen->Height+100);
    if (side == 1) *man = MakeSnowPerson(-100, RandomIntLessThan(Screen->Height));
    if (side == 3) *man = MakeSnowPerson(Screen->Width+100, RandomIntLessThan(Screen->Height));
}



internal void
LaunchSnowBall(rect sourceRect, int dir, float playerDx, float playerDy)
{
    int ballW = 14;
    int ballH = 14;
    int x = sourceRect.x + sourceRect.w/2 - ballW/2;
    int y = sourceRect.y + sourceRect.h/2 - ballH/2 + snowBallStartingHeight/2;

    snowballs[snowBallCounter].shadowPos = {x, y, ballW, ballH};
    snowballs[snowBallCounter].colPos = {x, y, ballW, ballH + snowBallStartingHeight};
    snowballs[snowBallCounter].dir = dir;
    snowballs[snowBallCounter].life = 0;                // time alive
    snowballs[snowBallCounter].height = snowBallStartingHeight; // pixels

    snowballs[snowBallCounter].xSpeed = playerDx / snowballAffectedByPlayerSpeedFactor;
    snowballs[snowBallCounter].ySpeed = playerDy / snowballAffectedByPlayerSpeedFactor;

    snowBallCounter = (snowBallCounter+1) % maxSnowBalls;
}

internal void 
ExplodeSnowBall(snow_ball *ball) 
{
    ball->life = -1;
    ball->height = 0;
    //particle here?
}


// global_variable int nextActiveSnowmanIndex;


internal void
AddActiveSnowManAtDir(snow_person *enemies, game_buffer *Screen, float distInFromEdge, int side)
{
    // if (nextActiveSnowmanIndex < MaxSnowmen)
    // {
    //     int i = nextActiveSnowmanIndex;

    //     if (side == 0) enemies[i] = MakeSnowPerson(RandomIntLessThan(Screen->Width), distInFromEdge);
    //     if (side == 2) enemies[i] = MakeSnowPerson(RandomIntLessThan(Screen->Width), Screen->Height-distInFromEdge);
    //     if (side == 1) enemies[i] = MakeSnowPerson(distInFromEdge, RandomIntLessThan(Screen->Height));
    //     if (side == 3) enemies[i] = MakeSnowPerson(Screen->Width-distInFromEdge, RandomIntLessThan(Screen->Height));
    
    //     nextActiveSnowmanIndex++;
    // }
    // else
    // {
    //     // added all we have,
    //     // increase snowman speed?
    // }

    bool foundOne = false;
    for (int i = 0; i < MaxSnowmen; i++)
    {
        if (enemies[i].allDead)
        {

            if (side == 0) enemies[i] = MakeSnowPerson(RandomIntLessThan(Screen->Width), distInFromEdge);
            if (side == 2) enemies[i] = MakeSnowPerson(RandomIntLessThan(Screen->Width), Screen->Height-distInFromEdge);
            if (side == 1) enemies[i] = MakeSnowPerson(distInFromEdge, RandomIntLessThan(Screen->Height));
            if (side == 3) enemies[i] = MakeSnowPerson(Screen->Width-distInFromEdge, RandomIntLessThan(Screen->Height));
    
            foundOne = true;
            break;
        }
    }
    if (!foundOne)
    {
        // added all we have,
        // increase snowman speed and reset timer, should feel like waves?
        SnowmanSpeed += 0.004f; 
        SnowmanSpawnTimerExtra = SnowmanSpawnTimerExtraDefault;
    }

}

internal void
AddActiveSnowMan(snow_person *enemies, game_buffer *Screen, float distInFromEdge)
{
    int side = RandomIntLessThan(4);
    AddActiveSnowManAtDir(enemies, Screen, distInFromEdge, side);
}




internal bool
SnowmanTouchesPlayer(snow_person man, rect player)
{
    float mcx = GetSnowManBaseCenterPosX(man);
    float mcy = GetSnowManBaseCenterPosY(man);

    float pcx = player.x + player.w/2;
    float pcy = player.y + player.h/2;


    float dist = sqrt((mcx-pcx)*(mcx-pcx) + 
                       (mcy-pcy)*(mcy-pcy));

    if (dist < PlayerTouchRadius)
        return true;
    else
        return false;
}



// internal void
// SnowmanJumpUp(snow_person *man, float dt)
// {
//     if (man->jumpUpLeft >= 0.0f)  // is jumping up
//     {
//         man->jumpUpLeft -= dt;

//         man->height += SnowmanJumpUpSpeed * dt;
//         ForceMoveEntireSnowMan(man, 0, -SnowmanJumpUpSpeed * dt);
//         // ForceMoveEntireSnowMan(man, 0, -SnowmanJumpUpSpeed*dt);
//         // man->height += SnowmanJumpUpSpeed*dt;
//     }
//     else  // way down or landed
//     {
//         // just landed
//         if (man->height <= 0.0f)// && !man->landed)
//         {
//             man->landed = true;
//             man->jumpUpLeft = SnowmanJumpLength; // will jump next frame
//         }
//     }

// }

const global_variable int maxDeadPieces = 200;
global_variable int deadPieceNextIndex;
global_variable int freedPieceIndex = 0; // should be <= nextIndex
global_variable dead_piece deadPieces[maxDeadPieces];


internal void
MakeDeadPiece(rect pos, float height)
{
    deadPieces[deadPieceNextIndex].pos = pos;
    deadPieces[deadPieceNextIndex].height = height;
    deadPieceNextIndex = (deadPieceNextIndex + 1) % maxDeadPieces;
    deadPieces[deadPieceNextIndex].isHat = false;
}
internal void
MakeDeadHatPiece(rect pos, float height)
{
    deadPieces[deadPieceNextIndex].pos = pos;
    deadPieces[deadPieceNextIndex].height = height;
    deadPieceNextIndex = (deadPieceNextIndex + 1) % maxDeadPieces;
    deadPieces[deadPieceNextIndex].isHat = true;
}



internal void
UpdateDeadPiece(dead_piece *piece, float dt)
{
    float manGrav = 0.20f;
    if (piece->height > 0.0f)
    {
        piece->height -= manGrav * dt;
        piece->pos.y += manGrav * dt;
        // SetAnimateMoveEntireSnowMan(man, 0, manGrav * dt);
    }
}



global_variable float secondSinceGameOver;
global_variable float requiredTimeOnGameOver = 2000.0f;

internal void
SetGameOver(int Score)
{
    GameOver = true;
    secondSinceGameOver = 0.0f;

    if (Score > SessionHighScore)
    {
        SessionHighScore = Score;
    }
}


internal void
ResetGame()
{
    GameOver = false;
    GameInitialized = false;
    secondSinceGameOver = 0.0f;
}


// internal float
// CountActiveSnowMen()
// {
//     float result = 0;
//     for (int i = 0; i < MaxSnowmen; i++)
//     {
//         if (!enemies[i].allDead) {
//             result++;
//         }
//     }
//     return result;
// }


internal void
MainGameLoop(game_buffer *Buffer, game_buffer *bgBuffer, game_input Input, 
             int *Score, char *Message, char *Message2, float dt)
{

    local_persist snow_person enemies[MaxSnowmen];

    local_persist rect playerRect;

    local_persist float TimeSinceInit = 0.0f;


    local_persist bool hitAllKeys;
    local_persist bool atLeastOneArrowKey;
    local_persist bool atLeastOneWASDKey;
    local_persist float tutTimeReq;

// dt/=2;
     // dt = 6.0f;

    if (GameOver)
    {
        secondSinceGameOver += dt;


        sprintf(Message, "SCORE: %d", *Score);



        // if (*Score == 0)
        //     sprintf(Message2, "The snowpeople love you for your peaceful ways.");
        // else
        //     sprintf(Message2, "Is that good or bad, who's to say?");


        if (secondSinceGameOver >= 1000.0f) 
        {
            // check for input to reset game
            // sprintf(Message2, "SESSION HIGH: %d", SessionHighScore);
            sprintf(Message2, "HIGH: %d", SessionHighScore);
        }

        if (secondSinceGameOver >= requiredTimeOnGameOver) 
        {

            if (Input.w ||
                Input.s ||
                Input.a ||
                Input.d ||
                Input.up ||
                Input.down ||
                Input.left ||
                Input.right)
            {
                ResetGame();
                *Score = 0;
                return;
            }
        }
    }

    //
    //
    // INIT
    if (!GameInitialized)
    {
        srand(time(NULL));

        TimeSinceInit = 0.0f;


        SnowmanSpawnTimerExtra = SnowmanSpawnTimerExtraDefault;
        SnowmanSpeed = SnowmanSpeedDefault;

        // *Message = "wasd";

        playerRect = {Buffer->Width/2, Buffer->Height/2, 32, 24};


        for (int i = 0; i < MaxSnowmen; i++) 
        {
            enemies[i] = MakeSnowPerson(-100, -100);
            enemies[i].allDead = true;
        }

        // start with 1 above 
        AddActiveSnowManAtDir(enemies, Buffer, -50, 3); // first is always to right
        SnowmanSpawnIn = SnowmanSpawnTimerBase + SnowmanSpawnTimerExtra; // slower first spawn
        // for (int i = 0; i < 3; i++) 
        // {
        //     AddActiveSnowMan(enemies, Buffer, (100*i)-200);
        // }

                    
        RenderBG(bgBuffer);



        for (int i = 0; i < maxDeadPieces; i++) 
        {
            deadPieceNextIndex = 0;
            deadPieces[i].pos = {-100, -100, 1, 1};
        }


        tutTimeReq = 5000.0f;
        hitAllKeys = false;
        atLeastOneArrowKey = false;
        atLeastOneWASDKey = false;


        GameInitialized = true;
    }



    if (!GameOver) 
    {


        // instructions...
        if (Input.up || Input.left || Input.down || Input.right) atLeastOneArrowKey = true;
        if (Input.w || Input.a || Input.s || Input.d) atLeastOneWASDKey = true;

        if (atLeastOneArrowKey && atLeastOneWASDKey && !hitAllKeys)
        {
            hitAllKeys = true;
           if (TimeSinceInit < tutTimeReq) tutTimeReq = TimeSinceInit+800.0f;
        }

        TimeSinceInit += dt;
        if (TimeSinceInit < tutTimeReq) 
            sprintf(Message, "WASD + arrows", SessionHighScore);




        SnowmanSpawnIn -= dt;
        if (SnowmanSpawnIn <= 0)
        {
            // if (SnowmanSpawnTimer > 500)
            //     SnowmanSpawnTimer -= 200/CountActiveSnowMen();
            SnowmanSpawnTimerExtra /= SnowmanSpawnTimerExtra;
            SnowmanSpawnIn = SnowmanSpawnTimerBase + SnowmanSpawnTimerExtra;
            AddActiveSnowMan(enemies, Buffer, -100);
        }

        // *Score = SnowmanSpawnIn; //test

        //
        //
        // PLAYER MOVEMENT


        float pdx = 0;
        float pdy = 0;
        if (Input.w) pdy = -1.0f * PlayerSpeed;
        if (Input.s) pdy =  1.0f * PlayerSpeed;
        if (Input.a) pdx = -1.0f * PlayerSpeed;
        if (Input.d) pdx =  1.0f * PlayerSpeed;

        // if (pdx != 0 && pdy != 0) PlayerSpeed *= 0.707f;

        playerRect.x += pdx * dt;// * PlayerSpeed * dt;
        playerRect.y += pdy * dt;// * PlayerSpeed * dt;


        //
        //
        // PLAYER ATTACK
        local_persist float launchCounter;
        if (launchCounter <=0)
        {
                 if (Input.up)    { LaunchSnowBall(playerRect, 1, pdx, pdy); launchCounter = launchDelay;}
            else if (Input.left)  { LaunchSnowBall(playerRect, 2, pdx, pdy); launchCounter = launchDelay;}
            else if (Input.down)  { LaunchSnowBall(playerRect, 3, pdx, pdy); launchCounter = launchDelay;}
            else if (Input.right) { LaunchSnowBall(playerRect, 4, pdx, pdy); launchCounter = launchDelay;}
        }
        else
        {
            launchCounter -= dt;
        }



        //
        //
        //
        // SNOWMEN FALLING OVER / DYING
        for (int i = 0; i < MaxSnowmen; i++)
        {
            if (enemies[i].allDead)
            {
                continue;
            }
            
            for (int j = 3; j >= 1; j--) // don't go to 0 cause we compare with one below
            {
                if (enemies[i].pieces[j].pos.x < enemies[i].pieces[j-1].pos.x - enemies[i].pieces[j-1].pos.w/2
                 || enemies[i].pieces[j].pos.x > enemies[i].pieces[j-1].pos.x + enemies[i].pieces[j-1].pos.w/2
                 || enemies[i].pieces[j].pos.y+enemies[i].pieces[j].pos.h < enemies[i].pieces[j-1].pos.y - enemies[i].pieces[j-1].pos.h/2
                 || enemies[i].pieces[j].pos.y+enemies[i].pieces[j].pos.h > enemies[i].pieces[j-1].pos.y + enemies[i].pieces[j-1].pos.h/2)
                {
                    // todo: fix this
                    // don't check mismatch with already dead pieces
                    if (!enemies[i].pieces[j].dead && !enemies[i].pieces[j-1].dead &&
                        enemies[i].delays[j] <= 0 && enemies[i].delays[j-1] <= 0 &&
                         enemies[i].tx[j] == 0 && enemies[i].ty[j] == 0)
                    {

                        if (j == 3)  // hat is off from head
                        {
                            // kill hat
                            enemies[i].hat.dead = true;
                            enemies[i].allDead;
                            // MakeDeadHatPiece(enemies[i].hat.pos, 0);
                            //man.height

                            (*Score)++;
                        }
                        if (j == 2)  // head is off from mid
                        {

                            // kill mid + below

                            if (!enemies[i].base.dead) {
                                enemies[i].height += enemies[i].base.pos.h;
                                enemies[i].base.dead = true;
                                MakeDeadPiece(enemies[i].base.pos, 0);
                            }

                            if (!enemies[i].mid.dead) {
                                enemies[i].height += enemies[i].base.pos.h;
                                enemies[i].mid.dead = true;
                                MakeDeadPiece(enemies[i].mid.pos, 0);
                            }

                        }
                        if (j == 1)  // mid is off from base
                        {
                            if (!enemies[i].base.dead) {
                                enemies[i].height += enemies[i].base.pos.h;
                                enemies[i].base.dead = true;
                                MakeDeadPiece(enemies[i].base.pos, 0);
                            }
                        }
                    }
                }
            }
            
            // move down if lower piece is destroyed

            // reset if all dead
            if (enemies[i].hat.dead || enemies[i].head.dead)
            {
                
                // bake into bg here
                // ..

                // for (int j = 0; j < 4; j++)
                // {
                //     if (!enemies[i].pieces[j].dead)
                //     {
                //         enemies[i].pieces[j].dead = true;
                //         if (j == 3) // hat
                //             MakeDeadHatPiece(enemies[i].pieces[j].pos, 0);
                //         else
                //             MakeDeadPiece(enemies[i].pieces[j].pos, 0);
                //     }
                // }

                if (!enemies[i].base.dead) {
                    // enemies[i].height += enemies[i].base.pos.h;
                    enemies[i].base.dead = true;
                    MakeDeadPiece(enemies[i].base.pos, 0);
                }
                if (!enemies[i].mid.dead) {
                    // enemies[i].height += enemies[i].mid.pos.h;
                    enemies[i].mid.dead = true;
                    MakeDeadPiece(enemies[i].mid.pos, 0);
                }
                if (!enemies[i].head.dead) {
                    // enemies[i].height += enemies[i].head.pos.h;
                    enemies[i].head.dead = true;
                    MakeDeadPiece(enemies[i].head.pos, 0);
                }
                if (!enemies[i].hat.dead) {
                    // enemies[i].height += enemies[i].hat.pos.h;
                    enemies[i].hat.dead = true;
                    // MakeDeadHatPiece(enemies[i].hat.pos, 0);
                }

                enemies[i].allDead = true;
                enemies[i].deadFor = 0.0f;


            }
        }

        // animate dead thigns, spaw nwe when done
        // for (int i = 0; i < MaxSnowmen; i++)
        // {
        //     if (enemies[i].allDead)
        //     {
        //         enemies[i].deadFor += dt;

        //         for (int p = 0; p < 4; p++)
        //         {
        //             if (enemies[i].pieces[p].dead)
        //             {
        //                 enemies[i].pieces[p].pos.y += (2.0f * dt);
        //             }
        //         }

        //         if (enemies[i].deadFor > 1.0f)
        //         {
        //             ForceMoveEntireSnowMan(&enemies[i], -3000, -3000); // prolly not needed?
        //             // ResetDeadSnowMan(&enemies[i], Buffer);
        //         }
        //     }
        // }



        // move towards player
        for (int i = 0; i < MaxSnowmen; i++)
        {
            if (!enemies[i].allDead) {

                float px = playerRect.x + playerRect.w/2;
                float py = playerRect.y + playerRect.h/2;
                float ax = GetSnowManBaseCenterPosX(enemies[i]);
                float ay = GetSnowManBaseCenterPosY(enemies[i]);

                float dist2p = sqrt(((px-ax)*(px-ax) + (py-ay)*(py-ay)));

                float tx = px;
                float ty = py;
                float speed = SnowmanSpeed;
                if (dist2p > (SnowmanStrikeRadius+5))
                {
                    tx = px + enemies[i].targetOffsetX;
                    ty = py + enemies[i].targetOffsetY;
                    speed = SnowmanStrikeSpeed;
                }
                speed *= enemies[i].randomSpeedFactor;

                float dist = sqrt(((tx-ax)*(tx-ax) + (ty-ay)*(ty-ay)));

                float ux = (tx-ax) / dist;
                float uy = (ty-ay) / dist;

                ForceMoveEntireSnowMan(&enemies[i], speed*ux*dt, speed*uy*dt);

            }
        }

        // player cols
        for (int i = 0; i < MaxSnowmen; i++)
        {
            if (!enemies[i].allDead)
            {
                if (SnowmanTouchesPlayer(enemies[i], playerRect))
                {
                    SetGameOver(*Score);
                }
            }
        }


    // apply grav / jump anim
    for (int i = 0; i < MaxSnowmen; i++)
    {
        if (!enemies[i].allDead) {
            // StepAnimateMoveEntireSnowMan(&enemies[i], dt);
            SnowManApplyGrav(&enemies[i], dt);
        }
    }


    }
    else  // on game over only...
    {
        for (int i = 0; i < MaxSnowmen; i++)
        {
            if (!enemies[i].allDead)
            {
                NewSnowManApplyGrav(&enemies[i], dt);
                // SnowmanJumpUp(&enemies[i], dt);
            }
        }
    }

    //
    //
    //
    // UPDATES THAT HAPPEN ALL THE TIME

    for (int i = 0; i < maxDeadPieces; i++)
    {
        if (deadPieces[i].height > 0)
        {
            UpdateDeadPiece(&deadPieces[i], dt);
        }
        else
        {
            BakeIntoBG(deadPieces[i].pos, bgBuffer);
        };
    }



    //
    //
    // SNOWBALLS

    // move
    for (int i = 0; i < maxSnowBalls; i++)
    {
        if (snowballs[i].life >= 0)
        {
            snowballs[i].life += dt;
            //rect drawRect = AddRects( snowballs[i].pos, {0,-snowballs[i].height,0,0} );


            float maxHeight = (float)snowBallStartingHeight;
            snowballs[i].height = (-powf(snowballs[i].life/maxDistFactor,6.0f)+maxHeight);

            if (snowballs[i].height <= 0)
            {
                snowballs[i].life = -1;
            }

            float ballDelta = ballSpeed * dt;
            
            rect dMove = {};
            if (snowballs[i].dir == 1) dMove = { 0, -ballDelta };
            if (snowballs[i].dir == 2) dMove = { -ballDelta, 0 };
            if (snowballs[i].dir == 3) dMove = { 0, ballDelta };
            if (snowballs[i].dir == 4) dMove = { ballDelta, 0 };

            float xExtra = (snowballs[i].xSpeed * dt);
            float yExtra = (snowballs[i].ySpeed * dt);
            dMove = { dMove.x+xExtra, dMove.y+yExtra, dMove.w, dMove.h };
            // dMove = AddRects(dMove, {(int)xExtra, (int)yExtra, 0, 0});

            snowballs[i].shadowPos = AddRects(snowballs[i].shadowPos, dMove);
            snowballs[i].colPos = AddRects(snowballs[i].shadowPos, {0,-snowballs[i].height,0,0});

        }
    }

    // collisions (could wrap this up with movement, that would prolly be better)
    for (int i = 0; i < maxSnowBalls; i++)
    {
        if (snowballs[i].life >= 0)
        {
            for (int j = 0; j < MaxSnowmen; j++)
            {
                int hit = SnowballHitsSnowman(snowballs[i], enemies[j]);
                if (hit > 0)
                {
                    int k = hit-1; // actual index of hit piece

					float thisNudge = nudgeAmount * 8.0f;
                    if (k == 3) thisNudge *= 2; // hat is one-hit kill

                    rect OffsetRect = {};
                    if (snowballs[i].dir == 1) OffsetRect = { 0, -thisNudge, 0, 0 };
                    if (snowballs[i].dir == 2) OffsetRect = { -thisNudge, 0, 0, 0 };
                    if (snowballs[i].dir == 3) OffsetRect = { 0,  thisNudge, 0, 0 };
                    if (snowballs[i].dir == 4) OffsetRect = {  thisNudge, 0, 0, 0 };

                    enemies[j].pieces[k].pos = AddRects(
                                                      enemies[j].pieces[k].pos,
                                                      OffsetRect
                                                      );

                    enemies[j].pieces[k].damage++;

                    ExplodeSnowBall(&snowballs[i]);
                }   
            }
        }
    }


    //
    //
    //
    // ---RENDER---
    //
    //
    //

    RenderBufferOnBuffer(bgBuffer, Buffer);


    //
    // SHADOWS

    // balls
    for (int i = 0; i < maxSnowBalls; i++)
    {
        if (snowballs[i].life >= 0)
        {
            FillRectInBuffer(snowballs[i].shadowPos, Buffer, SnowballShadow);
        }
    }

    //shadow people
    for (int i = 0; i < MaxSnowmen; i++)
    {
        enemies[i].shadowPos = CalcSnowManRect(enemies[i]);
        FillRectInBuffer(enemies[i].shadowPos, Buffer, SnowmanShadow);
    }


    //
    //
    // dead pieces

    for (int i = 0; i < maxDeadPieces; i++)
    {   
        // if (deadPieces[i].isHat)
        //     DrawRectAsTopHat(deadPieces[i].pos, Buffer, DeadHatColor);
        // else
            FillRectInBuffer(deadPieces[i].pos, Buffer, DeadPieceColor);
    }




    // TODO: put every rec in a list and sort it by Y before rendering?

    if (GameOver) // draw under everything if dead
        DrawDeadPlayer(playerRect, Buffer);

    // --draw things above player--
    for (int i = 0; i < MaxSnowmen; i++)
    {
        if (SnowManBasePosY(enemies[i]) < playerRect.y)
            DrawSnowMan(enemies[i], Buffer);
    }

    if (!GameOver)
        DrawPlayer(playerRect, Buffer);

    // --draw things below (in Y) player--
    for (int i = 0; i < MaxSnowmen; i++)
    {
        if (SnowManBasePosY(enemies[i]) >= playerRect.y)
            DrawSnowMan(enemies[i], Buffer);
    }




    //
    // SNOWBALLS
    for (int i = 0; i < maxSnowBalls; i++)
    {
        if (snowballs[i].life >= 0)
        {
            FillRectInBuffer(snowballs[i].colPos, Buffer, SnowballColor);
        }
    }


}
