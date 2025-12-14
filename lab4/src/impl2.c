#include <stdio.h>
#include "calc.h"

// Прямоугольный треугольник
float Square(float A, float B){
    return (A * B) / 2;
}

// Сортировка Хоара
void QuickSort(int* array, int low, int high) {
    if (low >= high) return;
    
    int pivot = array[(low + high) / 2];
    int i = low, j = high;
    
    while (i <= j) {
        while (array[i] < pivot) i++;
        while (array[j] > pivot) j--;
        if (i <= j) {
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
            i++;
            j--;
        }
    }
    
    QuickSort(array, low, j);
    QuickSort(array, i, high);
}

int* Sort(int* array, int size){
    QuickSort(array, 0, size - 1);
    return array;
}

