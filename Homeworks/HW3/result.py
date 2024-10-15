def readInputFile(fileName):
    with open(fileName, 'r') as f:
        f.readline()  # 跳過第一行數據長度
        numbers = list(map(int, f.readline().split()))
    return numbers

def writeOutputFile(fileName, sortedNumbers):
    with open(fileName, 'w') as f:
        for number in sortedNumbers:
            f.write(f'{number} ')

def sortNumbersFromFile(inputFileName, outputFileName):
    numbers = readInputFile(inputFileName)
    sortedNumbers = sorted(numbers)
    writeOutputFile(outputFileName, sortedNumbers)

def compareFiles(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        numbers1 = list(map(int, f1.readline().split()))
        numbers2 = list(map(int, f2.readline().split()))

    if numbers1 == numbers2:
        print(f"{file1} and {file2} are identical.")
    else:
        print(f"{file1} and {file2} are different.")

# 進行排序並輸出至 answer.txt
sortNumbersFromFile('input.txt', 'answer.txt')

# 比較 answer.txt 和 output_1.txt 到 output_8.txt
for i in range(1, 9):
    output_file = f'output_{i}.txt'
    compareFiles('answer.txt', output_file)

