## Instructions
First execute `MatrixOps_Server` and then execute `MatrixOps_Client`.

## Client Usage
`MatrixOps_Client [options] matrices...`

| Options | Description                             | #Matrices |
|---------|-----------------------------------------|-----------|
| mul     | Multipy Matrices                        | 2         |
| det     | Compute the Determinant of a NxN Matrix | 1         |
| inverse | Compute the Inverse of a NxN Matrix     | 1         |

### Examples

#### Multiplication
- `MatrixOps_Client mul "[[1,2][3,4]]" "[[9,8,7][6,5,4]]"`

    Expected Result: **`[[21,18,15][51,44,37]]`**

- `MatrixOps_Client mul "[[1,2,3][9,1,1][3,0,1][1,2,3]]" "[[1,2][3,4][5,6]]"`

    Expected Result: **`[[22,28][17,28][8,12][22,28]]`**

#### Determinant
- `MatrixOps_Client det "[[3,4,-7,6][1,2,-3,4][5,6,-7,5][-8,-9,1,2]]"`

    Expected Result: **8**

#### Inverse
- `MatrixOps_Client inverse "[[1,2,3][2,5,3][1,0,8]]"`

    Expected Result: **`[[-40,16,9][13,-5,-3][5,-2,-1]]`**

