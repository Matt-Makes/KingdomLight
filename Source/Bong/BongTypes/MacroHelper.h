#pragma once

// AI
#define BB_SET_BOOL(KeyName, Value) \
	GetBlackboardComponent()->SetValueAsBool(KeyName, Value);

#define BB_GET_BOOL(KeyName) \
	(GetBlackboardComponent()->GetValueAsBool(KeyName))

#define BB_SET_OBJECT(KeyName, Value) \
	GetBlackboardComponent()->SetValueAsObject(KeyName, Value);

#define BB_GET_OBJECT(KeyName) \
	(GetBlackboardComponent()->GetValueAsObject(KeyName))

#define BB_SET_VECTOR(KeyName, Value) \
	GetBlackboardComponent()->SetValueAsVector(KeyName, Value);

#define BB_GET_VECTOR(KeyName) \
	(GetBlackboardComponent()->GetValueAsVector(KeyName))


// Log
#define ACTOR_PTR_GET_SERVER_FSTRING(ActorPtr) \
	( (ActorPtr->HasAuthority()) ? TEXT("Server: ") : TEXT("Client: ") )



// Tool
#define BONG_MAX_NUMBER 6666






// Weapon
#define TRACE_LENGTH 80000.f