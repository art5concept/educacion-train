package main

import "fmt"

func (e email) cost() int {
	// ?
	if e.isSubscribed {
		return len(e.body) * 2
	} else {
		return len(e.body) * 5

	}
}

func (e email) format() string {
	// ?
	// var a string
	if e.isSubscribed {
		// a = "'" + e.body + "' | Subscribed"
		return fmt.Sprintf("'%s' | Subscribed", e.body)

	} else {
		// a = "'" + e.body + "' | Not Subscribed"
		return fmt.Sprintf("'%s' | Not Subscribed", e.body)
	}
	// return a

}

type expense interface {
	cost() int
}

type formatter interface {
	format() string
}

type email struct {
	isSubscribed bool
	body         string
}
