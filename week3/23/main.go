package main

type notification interface {
	importance() int
}

type directMessage struct {
	senderUsername string
	messageContent string
	priorityLevel  int
	isUrgent       bool
}

type groupMessage struct {
	groupName      string
	messageContent string
	priorityLevel  int
}

type systemAlert struct {
	alertCode      string
	messageContent string
}

func (dm directMessage) importance() int {
	if dm.isUrgent {
		return 50
	} else {
		return dm.priorityLevel
	}
}

func (gm groupMessage) importance() int {
	return gm.priorityLevel
}

func (sa systemAlert) importance() int {
	return 100
}

func processNotification(n notification) (string, int) {

	switch x := n.(type) {
	case directMessage:
		return x.senderUsername, x.importance()
	case groupMessage:
		return x.groupName, x.importance()
	case systemAlert:
		return x.alertCode, x.importance()
	default:
		return "", 0
	}
}
