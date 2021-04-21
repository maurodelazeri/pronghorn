# pronghorn
Zinnion DEFI streaming processor

# Queries

## Pancakeswap

```
{
  pairs(first: 1000, orderBy: reserveBNB, orderDirection: desc, 
  where: {reserve0_gt: 0, reserve1_gt: 0}) {
    id
    token0 {
      id
      name
      symbol
      derivedBNB
      decimals
    }
    token1 {
      id
      name
      symbol
      derivedBNB
      decimals
    }
    reserve0
    reserve1
    volumeToken0
    volumeToken1
    reserveBNB
    reserveUSD
    token0Price
    token1Price
  }
}
```
