package main

// func getExpenseReport(e expense) (string, float64) {
// 	// ?

// 	if  {

//		}
//	}

// por analizar para aprender
func getExpenseReport(e expense) (string, float64) {
	cost := e.cost() // Llama al método cost
	var report string

	// Usa un switch para determinar el tipo de expense
	switch e := e.(type) {
	// esta forma de usar
	case email:
		report = e.toAddress
	case sms:
		report = e.toPhoneNumber
	case invalid:
		report = ""
	}

	return report, cost // Devuelve el informe y el costo
}

// don't touch below this line

type expense interface {
	cost() float64
}

type email struct {
	isSubscribed bool
	body         string
	toAddress    string
}

type sms struct {
	isSubscribed  bool
	body          string
	toPhoneNumber string
}

type invalid struct{}

func (e email) cost() float64 {
	if !e.isSubscribed {
		return float64(len(e.body)) * .05
	}
	return float64(len(e.body)) * .01
}

func (s sms) cost() float64 {
	if !s.isSubscribed {
		return float64(len(s.body)) * .1
	}
	return float64(len(s.body)) * .03
}

func (i invalid) cost() float64 {
	return 0.0
}
