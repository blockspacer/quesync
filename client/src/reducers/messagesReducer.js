const messagesSort = (a, b) => {
	if (a.sentAt < b.sentAt)
		return -1;
	else if (a.sentAt > b.sentAt)
		return 1;
	else
		return 0;
}

const INITIAL_STATE = {
	messages: {},
	newMessages: {}
};

export default function reducer(
	state = INITIAL_STATE,
	action
) {
	switch (action.type) {
		case "SEND_MESSAGE_FULFILLED":
			var message = { ...action.payload.message };
			const channelId = message.channelId;

			const oldChannelMessages = state.messages[channelId] ? state.messages[channelId] : [];
			let channelMessages = [...oldChannelMessages];

			// Delete the channel id from the message
			delete message.channelId;

			// Add the message to the array
			channelMessages.push(message);

			// Sort the channel messages array
			channelMessages.sort(messagesSort);

			return { ...state, messages: { ...state.messages, [channelId]: channelMessages }, newMessages: { ...state.newMessages, [channelId]: null } };

		case "GET_CHANNEL_MESSAGES_FULFILLED":
			{
				let messages = [...action.payload.messages];

				const oldChannelMessages = state.messages[action.payload.channelId] ? state.messages[action.payload.channelId] : [];
				let channelMessages = [...oldChannelMessages];

				// For each message
				messages.forEach(message => {
					let messageCopy = { ...message }

					// Remove the channel id from the message
					delete messageCopy.channelId;

					// Add the message to the array of messages
					channelMessages.push(messageCopy);
				})

				// Sort the channel messages array
				channelMessages.sort(messagesSort);

				return { ...state, messages: { ...state.messages, [action.payload.channelId]: channelMessages } };
			}
		case "ADD_MESSAGE_TO_CHANNEL":
			{
				const oldChannelMessages = state.messages[action.payload.channelId] ? state.messages[action.payload.channelId] : [];
				let channelMessages = [...oldChannelMessages];

				// Add the message to the array
				channelMessages.push(action.payload.message);

				// Sort the channel messages array
				channelMessages.sort(messagesSort);

				return { ...state, messages: { ...state.messages, [action.payload.channelId]: channelMessages } };
			}

		case "SET_NEW_MESSAGE_CONTENT_FOR_CHANNEL":
			{
				const { channelId, content } = action.payload;

				return {
					...state, newMessages: {
						...state.newMessages,
						[channelId]:
							state.newMessages[channelId]
								? {
									...state.newMessages[channelId],
									content
								}
								: {
									content,
									attachmentId: null
								}
					}
				}
			}

		case "SET_NEW_MESSAGE_ATTACHMENT_FOR_CHANNEL":
			{
				const { channelId, attachmentId } = action.payload;

				return {
					...state, newMessages: {
						...state.newMessages,
						[channelId]:
							state.newMessages[channelId]
								? {
									...state.newMessages[channelId],
									attachmentId
								}
								: {
									content: null,
									attachmentId
								}
					}
				}
			}

		case "LOGOUT_FULFILLED":
		case "LOGOUT_REJECTED":
		case "CLEAN_CONNECTION":
			return INITIAL_STATE;

		default:
			return state;
	}
}
