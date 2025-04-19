package main

import "fmt"

func adder() func(int) int {
	var sum int
	sum = 0
	return func(number int) int {
		sum += number
		return sum
	}
}

func main() {
	add := adder()
	for i := 0; i <= 10; i++ {
		fmt.Println(add(i))
	}
}
