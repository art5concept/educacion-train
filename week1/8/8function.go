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
	fmt.Println(add(1))
	fmt.Println(add(2))
	fmt.Println(add(3))
}
