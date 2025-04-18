package main

import "fmt"

func main() {
	// initialize variables here
	var smsSendingLimit int
	var costPerSMS float64
	var hasPermission bool
	var username string

	fmt.Printf("%v %.2f %v %q\n", smsSendingLimit, costPerSMS, hasPermission, username)

	messageStart := "Happy birthday! You are now"
	age := 21
	messageEnd := "years old!"

	fmt.Println(messageStart, age, messageEnd)

	var username string = "presidentSkroob"
	var password string = "12345"

	// don't edit below this line
	fmt.Println("Authorization: Basic", username+":"+password)

	averageOpenRate, displayMessage := 0.23, "is the average open rate of your messages"
	fmt.Println(averageOpenRate, displayMessage)
}
