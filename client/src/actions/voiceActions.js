export function call(channelId) {
	return (dispatch, getState) => {
		const client = getState().client.client;

		return dispatch({
			type: "CALL",
			payload: client.voice().call(channelId)
		});
	}
}

export function joinCall(channelId) {
	return (dispatch, getState) => {
		const client = getState().client.client;
		const currentChannelId = getState().voice.channelId;

		// If the user is already in a call, clear it's current call
		if (currentChannelId) {
			dispatch({
				type: "CLEAR_CURRENT_CALL",
				payload: null
			});
		}

		// Join the new call
		dispatch({
			type: "JOIN_CALL",
			payload: client.voice().joinCall(channelId)
		});
	};
}

export function leaveCall() {
	return (dispatch, getState) => {
		const client = getState().client.client;

		return dispatch({
			type: "LEAVE_CALL",
			payload: client.voice().leaveCall()
		});
	}
}

export function updateVoiceState(userId, voiceState) {
	return {
		type: "UPDATE_VOICE_STATE",
		payload: {
			userId,
			voiceState
		}
	};
}

export function updateVoiceActivationState(userId, activated) {
	return {
		type: "UPDATE_VOICE_ACTIVATED_STATE",
		payload: {
			userId,
			activated
		}
	};
}

export function setVoiceState(mute, deafen) {
	return (dispatch, getState) => {
		const client = getState().client.client;

		return dispatch({
			type: "SET_VOICE_STATE",
			payload: client.voice().setVoiceState(mute, deafen)
		});
	}
}

export function addIncomingCall(call) {
	return {
		type: "ADD_INCOMING_CALL",
		payload: call
	};
}

export function setActiveCallInChannel(channelId) {
	return {
		type: "SET_ACTIVE_CALL_IN_CHANNEL",
		payload: channelId
	};
}

export function removeActiveCall(channelId) {
	return {
		type: "REMOVE_ACTIVE_CALL",
		payload: channelId
	};
}

export function getChannelCalls(channelId, amount, offset) {
	return (dispatch, getState) => {
		const client = getState().client.client;

		return dispatch({
			type: "GET_CHANNEL_CALLS",
			payload: client.voice().getChannelCalls(channelId, amount, offset)
		});
	}
}
