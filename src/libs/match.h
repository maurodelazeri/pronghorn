//
// Created by mauro on 4/8/21.
//

#pragma once

#include <cmath>
#include <iostream>

using namespace std;

/**********************************************************************************************
// calcSpotPrice                                                                             //
// sP = spotPrice                                                                            //
// bI = tokenBalanceIn                ( bI / wI )         1                                  //
// bO = tokenBalanceOut         sP =  -----------  *  ----------                             //
// wI = tokenWeightIn                 ( bO / wO )     ( 1 - sF )                             //
// wO = tokenWeightOut                                                                       //
// sF = swapFee                                                                              //
**********************************************************************************************/
// Fee incurring
// calc which would incur no slippage
inline double calcSpotPrice(double tokenBalanceIn,
                            double tokenWeightIn,
                            double tokenBalanceOut,
                            double tokenWeightOut,
                            double swapFee) {
    return ((tokenBalanceIn / tokenWeightIn) / (tokenBalanceOut / tokenWeightOut)) * (1 / (1 - swapFee));
}

// No fee incurring
// calc which would incur no slippage
inline double calcSpotPrice(double tokenBalanceIn,
                            double tokenWeightIn,
                            double tokenBalanceOut,
                            double tokenWeightOut) {
    return (tokenBalanceIn / tokenWeightIn) / (tokenBalanceOut / tokenWeightOut);
}

/**********************************************************************************************
// calcOutGivenIn                                                                            //
// aO = tokenAmountOut                                                                       //
// bO = tokenBalanceOut                                                                      //
// bI = tokenBalanceIn              /      /            bI             \    (wI / wO) \      //
// aI = tokenAmountIn    aO = bO * |  1 - | --------------------------  | ^            |     //
// wI = tokenWeightIn               \      \ ( bI + ( aI * ( 1 - sF )) /              /      //
// wO = tokenWeightOut                                                                       //
// sF = swapFee                                                                              //
**********************************************************************************************/
inline double calcOutGivenIn(double tokenBalanceIn,
                             double tokenWeightIn,
                             double tokenBalanceOut,
                             double tokenWeightOut,
                             double tokenAmountIn,
                             double swapFee) {
    // https://eth-converter.com/
    // https://etherscan.io/address/0x59a19d8c652fa0284f44113d0ff9aba70bd46fb4#readContract
    //tokenBalanceIn (uint256) 22448802480829414329729
    //tokenWeightIn (uint256) 10
    //tokenBalanceOut (uint256) 3668487681401719426107145
    //tokenWeightOut (uint256) 40
    //tokenAmountIn (uint256) 10000000000000000000
    //swapFee (uint256) 1500000000000000
    // Result:
    //tokenAmountOut   uint256 :  407813179110242470447
    double result = 1 - pow(tokenBalanceIn / (tokenBalanceIn + (tokenAmountIn * (1 - swapFee))),
                            tokenWeightIn / tokenWeightOut);
    return tokenBalanceOut * result;
}

/**********************************************************************************************
// calcInGivenPrice                                                                          //
// cSP = currentPrice                                                                        //
// dSP = desiredPrice                                                                        //
// wI = tokenWeightIn              /     /      cSP        \      (wO)            \          //
// wO = tokenWeightOut   ai = bi * |     | ---------------- | ^  ---------    -1   |         //
// aI = tokenAmountIn              \     \      dSP       /      (w0+wI)          |          //
// bI = tokenBalanceIn                                                                       //
**********************************************************************************************/
inline double calcInGivenPrice(double currentPrice,
                               double desiredPrice,
                               double tokenWeightIn,
                               double tokenWeightOut,
                               double tokenBalanceIn) {
    double result = pow((desiredPrice / currentPrice), (tokenWeightOut / (tokenWeightIn + tokenWeightOut))) - 1;
    result = tokenBalanceIn * result;
    if (result < 0)
        result = result * -1;
    return result;
}

/**********************************************************************************************
// calcInGivenOut                                                                            //
// aI = tokenAmountIn                                                                        //
// bO = tokenBalanceOut               /  /     bO      \    (wO / wI)      \                 //
// bI = tokenBalanceIn          bI * |  | ------------  | ^            - 1  |                //
// aO = tokenAmountOut    aI =        \  \ ( bO - aO ) /                   /                 //
// wI = tokenWeightIn           --------------------------------------------                 //
// wO = tokenWeightOut                          ( 1 - sF )                                   //
// sF = swapFee                                                                              //
**********************************************************************************************/
inline double calcInGivenOut(double tokenBalanceIn,
                             double tokenWeightIn,
                             double tokenBalanceOut,
                             double tokenWeightOut,
                             double tokenAmountOut,
                             double swapFee) {
    return 0.0;
}

/**********************************************************************************************
// calcPoolOutGivenSingleIn                                                                  //
// pAo = poolAmountOut         /                                              \              //
// tAi = tokenAmountIn        ///      /     //    wI \      \\       \     wI \             //
// wI = tokenWeightIn        //| tAi *| 1 - || 1 - --  | * sF || + tBi \    --  \            //
// tW = totalWeight     pAo=||  \      \     \\    tW /      //         | ^ tW   | * pS - pS //
// tBi = tokenBalanceIn      \\  ------------------------------------- /        /            //
// pS = poolSupply            \\                    tBi               /        /             //
// sF = swapFee                \                                              /              //
**********************************************************************************************/


/**********************************************************************************************
// calcSingleInGivenPoolOut                                                                  //
// tAi = tokenAmountIn              //(pS + pAo)\     /    1    \\                           //
// pS = poolSupply                 || ---------  | ^ | --------- || * bI - bI                //
// pAo = poolAmountOut              \\    pS    /     \(wI / tW)//                           //
// bI = balanceIn          tAi =  --------------------------------------------               //
// wI = weightIn                              /      wI  \                                   //
// tW = totalWeight                          |  1 - ----  |  * sF                            //
// sF = swapFee                               \      tW  /                                   //
**********************************************************************************************/


/**********************************************************************************************
// calcSingleOutGivenPoolIn                                                                  //
// tAo = tokenAmountOut            /      /                                             \\   //
// bO = tokenBalanceOut           /      // pS - (pAi * (1 - eF)) \     /    1    \      \\  //
// pAi = poolAmountIn            | bO - || ----------------------- | ^ | --------- | * b0 || //
// ps = poolSupply                \      \\          pS           /     \(wO / tW)/      //  //
// wI = tokenWeightIn      tAo =   \      \                                             //   //
// tW = totalWeight                    /     /      wO \       \                             //
// sF = swapFee                    *  | 1 - |  1 - ---- | * sF  |                            //
// eF = exitFee                        \     \      tW /       /                             //
**********************************************************************************************/


/**********************************************************************************************
// calcPoolInGivenSingleOut                                                                  //
// pAi = poolAmountIn               // /               tAo             \\     / wO \     \   //
// bO = tokenBalanceOut            // | bO - -------------------------- |\   | ---- |     \  //
// tAo = tokenAmountOut      pS - ||   \     1 - ((1 - (tO / tW)) * sF)/  | ^ \ tW /  * pS | //
// ps = poolSupply                 \\ -----------------------------------/                /  //
// wO = tokenWeightOut  pAi =       \\               bO                 /                /   //
// tW = totalWeight           -------------------------------------------------------------  //
// sF = swapFee                                        ( 1 - eF )                            //
// eF = exitFee                                                                              //
**********************************************************************************************/