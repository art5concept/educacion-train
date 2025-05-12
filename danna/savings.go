package main

import (
	"fmt"
	"time"
)

type Deposito struct {
	Fecha time.Time
	Monto float64
}

func generarDepositosLunes(desde, hasta time.Time, monto float64) []Deposito {
	var depositos []Deposito
	diaActual := desde

	// Avanza hasta el primer lunes
	for diaActual.Weekday() != time.Monday {
		diaActual = diaActual.AddDate(0, 0, 1)
	}

	// Agrega $10 cada lunes
	for !diaActual.After(hasta) {
		depositos = append(depositos, Deposito{
			Fecha: diaActual,
			Monto: monto,
		})
		diaActual = diaActual.AddDate(0, 0, 7) // siguiente lunes
	}

	return depositos
}

func esMartesPrimeraOTerceraSemana(fecha time.Time) bool {
	if fecha.Weekday() != time.Tuesday {
		return false
	}
	dia := fecha.Day()
	return (dia >= 1 && dia <= 7) || (dia >= 15 && dia <= 21)
}

func simulateSavingsAccount(
	fechaInicio time.Time,
	fechaFin time.Time,
	saldoInicial float64,
	tasaAnual float64,
	depositos []Deposito,
) {
	saldo := saldoInicial
	tasaDiaria := tasaAnual / 365
	interesAcumulado := 0.0
	fechaActual := fechaInicio
	depositoMap := make(map[string]float64)

	for _, dep := range depositos {
		key := dep.Fecha.Format("2006-01-02")
		depositoMap[key] += dep.Monto
	}

	for !fechaActual.After(fechaFin) {
		key := fechaActual.Format("2006-01-02")

		// Aplicar depósito si corresponde
		if monto, ok := depositoMap[key]; ok {
			saldo += monto
		}

		// Calcular interés diario
		interesDiario := saldo * tasaDiaria
		interesAcumulado += interesDiario

		// Imprimir saldo si es martes de la primera o tercera semana
		if esMartesPrimeraOTerceraSemana(fechaActual) {
			fmt.Printf("[Saldo %s] $%.2f\n", fechaActual.Format("02/01/2006"), saldo)
		}

		// Capitalización mensual
		siguienteDia := fechaActual.AddDate(0, 0, 1)
		if siguienteDia.Month() != fechaActual.Month() {
			saldo += interesAcumulado
			fmt.Printf("🧾 Fin de %02d/%d - Saldo: $%.2f - Interés mensual: $%.2f\n",
				fechaActual.Month(), fechaActual.Year(), saldo, interesAcumulado)
			interesAcumulado = 0.0
		}

		fechaActual = siguienteDia
	}

	fmt.Printf("\n🏁 Saldo final al %s: $%.2f\n", fechaFin.Format("02/01/2006"), saldo)
}

func main() {
	fechaInicio := time.Date(2025, 5, 25, 0, 0, 0, 0, time.UTC)
	fechaFin := time.Date(2026, 12, 25, 0, 0, 0, 0, time.UTC)

	depositos := generarDepositosLunes(fechaInicio, fechaFin, 160.0)

	simulateSavingsAccount(
		fechaInicio,
		fechaFin,
		100.0,
		0.0425,
		depositos,
	)
}
