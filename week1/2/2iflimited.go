package main

import "fmt"

func getLength(s string) int {
	return len(s)
}

func main() {

	email := "hello world!"

	if length := getLength(email); length < 10 {
		fmt.Println("Email is valid")
	} else {
		fmt.Println("Email is invalid")
	}
}
