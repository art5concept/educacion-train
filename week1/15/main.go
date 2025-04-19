package main

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
