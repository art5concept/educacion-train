package main

import "fmt"

type User struct {
	Name string
	Membership
}

type Membership struct {
	Type             string
	MessageCharLimit int
}

func newUser(name string, membershipType string) User {
	var messageCharLimit int
	if membershipType == "premium" {
		messageCharLimit = 1000
	} else {
		messageCharLimit = 100
	}
	user := User{
		Name: name,
		Membership: Membership{
			Type:             membershipType,
			MessageCharLimit: messageCharLimit,
		},
	}

	return user
}

func main() {
	// Create a new user
	user := newUser("Alice", "Premium")

	// Print user details
	fmt.Printf("User Name: %s\n", user.Name)
	fmt.Printf("Membership Type: %s\n", user.Type)                     // Accessing embedded field directly
	fmt.Printf("Message Character Limit: %d\n", user.MessageCharLimit) // Accessing embedded field directly
}
