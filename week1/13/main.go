package main

type authenticationInfo struct {
	username string
	password string
}

// create the method below
func (x authenticationInfo) getBasicAuth() string {
	a := x.username
	b := x.password
	c := "Authorization: Basic " + a + ":" + b
	return c
}
