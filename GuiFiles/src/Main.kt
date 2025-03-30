import javafx.application.Application
import javafx.application.Platform
import javafx.geometry.Pos
import javafx.scene.Scene
import javafx.scene.control.*
import javafx.scene.image.Image
import javafx.scene.image.ImageView
import javafx.scene.layout.HBox
import javafx.scene.layout.VBox
import javafx.stage.Modality
import javafx.scene.text.Text
import javafx.scene.text.TextAlignment
import javafx.stage.FileChooser
import javafx.stage.Stage
import java.io.FileInputStream
import java.io.FileNotFoundException
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import kotlin.system.exitProcess

class MainApp : Application()
{
    private val pipe = Pipe()
    @Volatile private var userId = 0
    @Volatile private var currUser = "None"
    @Volatile private var currAlbum = "None"
    @Volatile private var albums = "None"
    @Volatile private var usersList = mutableListOf<String>()
    @Volatile private var topTaggedImagePath = "C:/GalleryGUI/Images/top1.png"
    @Volatile private var topTaggedImageName = "None"
    @Volatile private var mostTaggedUser = "None"

    override fun start(primaryStage: Stage)
    {
        //starting the pipe
        pipe.pipeInit()
        pipe.sendMessage("6:None")
        val root = VBox().apply {
            spacing = 5.0
            alignment = Pos.CENTER
        }

        //creating lables and containers for each
        val titleText = createText("Omer's Gallery",  -20.0, "-fx-font-size: 34px; -fx-font-weight: bold; -fx-fill: lightskyblue;")
        val albumText = createText("$currUser: $currAlbum",  -30.0, "-fx-font-size: 28px; -fx-font-weight: bold;")
        val usersListText = createScrollableUsersList()
        val albumsListText = createText("$albums", -30.0, "-fx-font-size: 16px; -fx-font-weight: bold; -fx-fill: cornflowerblue;")
        val scrollableImages = createImageScroller()
        val firstSeparator = Separator()
        val secondSeparator = Separator()
        val openAlbumButton = createStyledButton("Open Album", 2)
        val closeAlbumButton = createStyledButton("Close Album",  3)
        val openUserButton = createStyledButton("Open User", 16)
        val closeUserButton = createStyledButton("Close User",  5)
        val addUserButton = createStyledButton("Add User", 14)
        val addPictureButton = createStyledButton("Add Picture", 7)
        val addAlbumButton = createStyledButton("Add Album", 1)
        val addTagButton = createStyledButton("Add Tag", 11)
        val removeUserButton = createStyledButton("Remove User", 15)
        val removePictureButton = createStyledButton("Remove Picture",8)
        val removeAlbumButton = createStyledButton("Remove Album", 4)
        val removeTagButton = createStyledButton("Remove Tag", 12)


        //containers
        val topTaggedImageLabel = Text(topTaggedImageName).apply {
            style = "-fx-font-size: 16px; -fx-font-weight: bold; -fx-fill: #5cb3ed;"
        }

        val topTagUserLabel = Text("Most Tagged: $mostTaggedUser").apply {
            style = "-fx-font-size: 12px; -fx-font-weight: bold; -fx-fill: gray;"
        }

        val topTaggedPicture = createImage(topTaggedImagePath, 200.0, 250.0)

        val topTaggedBox = VBox(topTaggedImageLabel, topTaggedPicture, topTagUserLabel).apply {
            alignment = Pos.CENTER
            spacing = 5.0
        }

        startMessageListener(albumText, usersListText, albumsListText, topTaggedImageLabel, topTaggedPicture, topTagUserLabel, scrollableImages)





        val buttonsContainer = HBox(10.0, addUserButton, addPictureButton, addAlbumButton, addTagButton, removeUserButton, removePictureButton, removeAlbumButton, removeTagButton).apply {
            alignment = Pos.CENTER
        }
        val buttonSecondContainer = HBox(10.0, openAlbumButton, closeAlbumButton, openUserButton, closeUserButton).apply {
            alignment = Pos.CENTER
        }
        val textContainer = VBox(titleText, albumText, usersListText, albumsListText).apply {
            alignment = Pos.TOP_CENTER
            spacing = 0.0
        }

        val pictureRow = HBox(20.0, topTaggedBox, scrollableImages).apply {
            alignment = Pos.CENTER
        }

        val pictureContainer = VBox(firstSeparator, pictureRow, secondSeparator)


        //margins
        VBox.setMargin(albumText, javafx.geometry.Insets(5.0, 0.0, 5.0, 0.0))
        VBox.setMargin(scrollableImages, javafx.geometry.Insets(10.0, 0.0, 10.0, 0.0))
        root.padding = javafx.geometry.Insets(20.0)

        root.children.addAll(textContainer, pictureContainer, buttonsContainer, buttonSecondContainer)

        val scene = Scene(root, 1000.0, 600.0)
        //get the title with time
        val formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm")
        val current = LocalDateTime.now().format(formatter)
        primaryStage.icons.add(Image("file:C:/GalleryGUI/Images/GalleryIcon.png"))
        primaryStage.title = "Omer's Gallery - $current"
        primaryStage.isResizable = false
        primaryStage.scene = scene
        primaryStage.show()
        primaryStage.setOnCloseRequest {
            pipe.close()
            exitProcess(0)
        }
    }

    private fun startMessageListener(albumText: Text, usersListText: ScrollPane, albumListText: Text, topTaggedImageLabel: Text, topTaggedPicture: ImageView, topTagUserLabel: Text, imageScroller: ScrollPane) {
        val executor = Executors.newSingleThreadScheduledExecutor()
        //getting thee messages
        executor.scheduleAtFixedRate({
            val message = pipe.readMessage()
            if (message != null) {
                println("Received: $message")
                Platform.runLater {
                    val parts = message.split(",")
                    if (parts.isNotEmpty()) {
                        val command = parts[0].toIntOrNull()

                        when (command)
                        {
                            //for each message that we get we will handle it
                            6 -> if (parts.size >= 4) {
                                topTaggedImagePath = parts[1]
                                topTaggedImageName = parts[3]
                                mostTaggedUser = parts[2]

                                topTaggedImageLabel.text = topTaggedImageName
                                topTaggedPicture.image = createImage(topTaggedImagePath, 200.0, 250.0).image
                                topTagUserLabel.text = "Most Tagged: $mostTaggedUser"
                            }
                            1 -> if (parts.size == 3) {
                                val newUserId = parts[1].toIntOrNull()
                                val newUserName = parts[2]
                                if (newUserId != null) {
                                    updateAlbumsList("None", albumListText, true)
                                    updateUser(newUserId, newUserName, albumText)

                                }
                            }
                            4 -> if (parts.size == 1) {
                                usersList.clear()
                                val textNode = (usersListText.content as VBox).children[0] as Text
                                textNode.text = ""
                            }
                            10 -> if(parts.isNotEmpty())
                            {
                                val imageContainer = imageScroller.content as HBox
                                imageContainer.children.clear()
                            }
                            11 -> if(parts.isNotEmpty())
                            {
                                albums = "None"
                            }
                            14 -> if (parts.size == 2) {
                                val addedUserName = parts[1]
                                updateUsersList(addedUserName, usersListText, true)
                            }
                            15 -> if (parts.size == 3) {
                                val removedUserName = parts[2]
                                updateUsersList(removedUserName, usersListText, false)
                            }
                            //open album
                            5 -> if (parts.size >= 3) {
                                val newAlbum = parts[1]
                                updateAlbumName(newAlbum, albumText)
                                val images = listOf(Pair(parts[3], parts[4]))
                                updateImageScroller(imageScroller, images)
                            }
                            //close User
                            9 -> if(parts.size == 1)
                            {
                                val newAlbum = "None"
                                updateAlbumName(newAlbum, albumText)
                                updateAlbumsList("None", albumListText, true)
                                (imageScroller.content as HBox).children.clear()
                            }
                            12 -> if (parts.size >= 5) {
                                val userTag = parts[3]
                                val pictureName = parts[4]

                                // find the picture in the image scroller and update its tag
                                val imageContainer = imageScroller.content as HBox
                                for (i in 0 until imageContainer.children.size) {
                                    val imageBox = imageContainer.children[i] as VBox
                                    val nameLabel = imageBox.children[0] as Text

                                    if (nameLabel.text == pictureName) {
                                        val tagScroller = imageBox.children[2] as ScrollPane
                                        val tagLabel = tagScroller.content as Text

                                        val currentTag = tagLabel.text
                                        val newTag = if (currentTag == "No tags") {
                                            "@$userTag"
                                        } else {
                                            "$currentTag, @$userTag"
                                        }

                                        tagLabel.text = newTag
                                        break
                                    }
                                }
                            }
                            3 -> if(parts.size == 1)
                            {
                                val newAlbum = "None"
                                updateAlbumName(newAlbum, albumText)
                                (imageScroller.content as HBox).children.clear()
                            }
                            2 -> if (parts.size > 2) {
                                val albumName = parts[2]
                                updateAlbumsList(albumName, albumListText, false)
                            }
                            13 -> if (parts.size >= 2) {
                                val pictureName = parts[1]

                                val imageContainer = imageScroller.content as HBox
                                for (i in 0 until imageContainer.children.size) {
                                    val imageBox = imageContainer.children[i] as VBox
                                    val nameLabel = imageBox.children[0] as Text

                                    if (nameLabel.text == pictureName) {
                                        val tagScroller = imageBox.children[2] as ScrollPane
                                        val tagLabel = tagScroller.content as Text
                                        tagLabel.text = "No tags"
                                        break
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }, 0, 100, TimeUnit.MILLISECONDS)
    }
    //scroll user list
    private fun createScrollableUsersList(): ScrollPane {
        val usersListText = Text("No Users").apply {
            style = "-fx-font-size: 16px; -fx-font-weight: bold; -fx-fill: cornflowerblue; -fx-text-alignment: center;"
            textAlignment = TextAlignment.CENTER
        }

        return ScrollPane(usersListText).apply {
            prefWidth = 200.0
            prefHeight = 100.0
            isFitToWidth = true
            translateY = -30.0
            hbarPolicy = ScrollPane.ScrollBarPolicy.NEVER
            vbarPolicy = ScrollPane.ScrollBarPolicy.AS_NEEDED
            style = "-fx-background-color: transparent;"

            val container = VBox(usersListText).apply {
                alignment = Pos.CENTER
                style = "-fx-background-color: transparent;"
            }
            content = container
        }
    }
    //updates the album list
    private fun updateAlbumsList(albumName: String, albumText: Text, reset : Boolean) {
        Platform.runLater {
            if(reset)
            {
                albums = "None"
                albumText.text = "None"
            }
            if (albums == "None" || albums.isEmpty()) {
                albums = albumName
            } else {
                albums += ", $albumName"
            }
            albumText.text = albums

        }
    }

    //update the current user
    private fun updateUser(newUserId: Int, newUserName: String, albumText: Text)
    {
        Platform.runLater {
            currUser = newUserName
            userId = newUserId
            albumText.text = "@$userId - $currUser: $currAlbum"
        }
    }
    //updates user list
    private fun updateUsersList(userName: String, usersListText: ScrollPane, isAdding: Boolean)
    {
        Platform.runLater {
            if (isAdding) {
                    usersList.add(userName)

            } else {
                usersList.remove(userName)
            }

            val usersString = if (usersList.isEmpty()) "No Users" else usersList.joinToString(", ")
            val textNode = (usersListText.content as VBox).children[0] as Text
            textNode.text = usersString
        }
    }
    //updates the album name
    private fun updateAlbumName(newAlbum: String, albumText: Text)
    {
        Platform.runLater {
            currAlbum = newAlbum
            albumText.text = "@$userId - $currUser: $currAlbum"
        }
    }
    //show add picture dialog
    //shows the add picture
    private fun showAddPictureDialog(): String? {
        val dialogStage = Stage()
        dialogStage.initModality(Modality.APPLICATION_MODAL)
        dialogStage.title = "Add Picture"

        val nameField = TextField().apply { promptText = "Enter Picture Name" }

        val fileChooserButton = Button("Choose File")
        val filePathLabel = Label("No file selected").apply { style = "-fx-text-fill: gray;" }

        var selectedFilePath: String? = null

        fileChooserButton.setOnAction {
            val fileChooser = FileChooser().apply {
                title = "Select Picture"
                extensionFilters.add(FileChooser.ExtensionFilter("Image Files", "*.png", "*.jpg", "*.jpeg"))
            }
            val selectedFile = fileChooser.showOpenDialog(dialogStage)
            if (selectedFile != null) {
                selectedFilePath = selectedFile.absolutePath
                filePathLabel.text = selectedFile.name
            }
        }

        val confirmButton = Button("Confirm").apply {
            setOnAction {
                val pictureName = nameField.text.trim()
                if (pictureName.isEmpty() || selectedFilePath == null) {
                    showErrorDialog("Invalid Input", "Please enter a name and select a file.")
                } else {
                    dialogStage.userData = "$pictureName,$selectedFilePath"
                    dialogStage.close()
                }
            }
        }

        val cancelButton = Button("Cancel").apply {
            setOnAction {
                dialogStage.userData = null
                dialogStage.close()
            }
        }

        val buttonBox = HBox(10.0, confirmButton, cancelButton).apply {
            alignment = Pos.CENTER
        }

        val layout = VBox(10.0, Label("Enter Picture Name:"), nameField, fileChooserButton, filePathLabel, buttonBox).apply {
            alignment = Pos.CENTER
            style = "-fx-padding: 20px;"
        }

        dialogStage.scene = Scene(layout, 400.0, 200.0)
        dialogStage.showAndWait()

        return dialogStage.userData as? String
    }
    private fun createImageScroller(): ScrollPane
    {
        val imageContainer = HBox(10.0)
        imageContainer.alignment = Pos.CENTER

        return ScrollPane(imageContainer).apply {
            isFitToHeight = true
            isFitToWidth = true
            prefHeight = 300.0
            style = "-fx-background-color: transparent; -fx-border-color: transparent;"
            hbarPolicy = ScrollPane.ScrollBarPolicy.NEVER
            vbarPolicy = ScrollPane.ScrollBarPolicy.NEVER
        }
    }
    //image scroller
    private fun updateImageScroller(scrollPane: ScrollPane, imagePaths: List<Pair<String, String>>)
    {
        Platform.runLater {
            val imageContainer = scrollPane.content as HBox

            for ((imagePath, imageName) in imagePaths)
            {
                val imageView = createImage(imagePath, 200.0, 250.0)

                val pictureLabel = Text(imageName).apply {
                    style = "-fx-font-size: 14px; -fx-font-weight: bold; -fx-fill: #5cb3ed;"
                }

                val tagLabel = Text("No tags").apply {
                    style = "-fx-font-size: 12px; -fx-font-weight: bold; -fx-fill: gray;"
                }

                val tagScroller = ScrollPane(tagLabel).apply {
                    prefWidth = 200.0
                    isFitToWidth = true
                    hbarPolicy = ScrollPane.ScrollBarPolicy.NEVER
                    vbarPolicy = ScrollPane.ScrollBarPolicy.NEVER
                    style = "-fx-background-color: transparent; -fx-border-color: transparent;"
                }

                val imageBox = VBox(pictureLabel, imageView, tagScroller).apply {
                    alignment = Pos.CENTER
                    spacing = 5.0
                }

                imageContainer.children.add(imageBox)
            }
        }
    }
    private fun createStyledButton(text: String, buttonId: Int): Button {
        return Button(text).apply {
            val buttonStyle = """
        -fx-background-color: linear-gradient(to bottom, #dbf1ff, #dbf1ff); 
        -fx-text-fill: black; 
        -fx-font-size: 14px; 
        -fx-padding: 8px 14px; 
        -fx-background-radius: 5px; 
        -fx-border-radius: 5px;
        -fx-border-color: transparent;
    """.trimIndent()

            val buttonHoverStyle = """
        -fx-background-color: linear-gradient(to bottom, #d6efff, #d6efff);
        -fx-cursor: hand;
    """.trimIndent()

            style = buttonStyle
            setOnMouseEntered { this.style = buttonStyle + buttonHoverStyle }
            setOnMouseExited { this.style = buttonStyle }
            setOnMouseClicked {
                val userInput: String? = when (buttonId) {
                    11 -> showMultiInputDialog("Add Tag", listOf("User ID to Add Tag", "Picture Name")) // remove Tag
                    7 -> showAddPictureDialog()
                    12 -> showMultiInputDialog("Remove Tag", listOf("User ID to Add Tag", "Picture Name")) // remove Tag
                    16, 15 -> showCustomInputDialog("User ID")
                    3, 5 -> closeCommand(buttonId)
                    else -> showCustomInputDialog(text)
                }
                if (!userInput.isNullOrEmpty())
                {
                    if(userInput != "None")
                    {
                        pipe.sendMessage("$buttonId:$userId,$userInput")
                    }
                    else
                    {
                        pipe.sendMessage("$buttonId:None")
                    }
                }
                else
                {
                    showErrorDialog("invalid Input", "Please enter a valid name")
                }
            }
        }
    }
    //error dialog
    private fun showErrorDialog(title: String, message: String)
    {
        val alert = Alert(Alert.AlertType.ERROR)
        alert.title = title
        alert.headerText = null
        alert.contentText = message
        alert.showAndWait()
    }
    //closing command
    private fun closeCommand(buttonId : Int) : String
    {
        if(buttonId == 5)
        {
            currUser = "None"
            userId = 0
            currAlbum = "None"
        }
        else
        {
            currAlbum = "None"
        }
        return "None"
    }
    //creating an image
    private fun createImage(filePath: String, width: Double, height: Double): ImageView
    {
        val imageView = try
        {
            val inputStream = FileInputStream(filePath)
            val image = Image(inputStream)
            ImageView(image)
        } catch (e: FileNotFoundException)
        {
            println("Image not found!")
            ImageView()
        }
        imageView.fitHeight = height
        imageView.fitWidth = width

        return imageView
    }
    //show custom input dialog
    private fun showCustomInputDialog(prompt: String): String?
    {
        val dialogStage = Stage()
        dialogStage.initModality(Modality.APPLICATION_MODAL)
        dialogStage.title = "$prompt Name"

        val inputField = TextField()
        inputField.promptText = "Enter $prompt name..."

        val errorLabel = Label().apply {
            style = "-fx-text-fill: red; -fx-font-size: 12px;"
            isVisible = false
        }

        val confirmButton = Button("Confirm").apply {
            setOnAction {
                val userInput = inputField.text.trim()
                if (userInput.isEmpty())
                {
                    errorLabel.text = "⚠️ Please enter a valid name."
                    errorLabel.isVisible = true
                } else {
                    dialogStage.userData = userInput
                    dialogStage.close()
                }
            }
        }

        val cancelButton = Button("Cancel").apply {
            setOnAction {
                dialogStage.userData = null
                dialogStage.close()
            }
        }

        inputField.setOnAction { confirmButton.fire() }

        val buttonBox = HBox(10.0, confirmButton, cancelButton).apply {
            alignment = Pos.CENTER
        }

        val layout = VBox(10.0, Label("Enter $prompt Name:"), inputField, errorLabel, buttonBox).apply {
            alignment = Pos.CENTER
            style = "-fx-padding: 20px;"
        }

        dialogStage.scene = Scene(layout, 300.0, 150.0)
        dialogStage.showAndWait()

        return dialogStage.userData as? String
    }
    //creating a text
    private fun createText(text : String, y : Double, style : String): Text
    {
        val textPanel = Text()
        textPanel.text = text
        textPanel.translateY = y
        textPanel.style = style
        return textPanel
    }
    //dialog with 2 choices
    private fun showMultiInputDialog(prompt: String, fields: List<String>): String?
    {
    val dialogStage = Stage()
    dialogStage.initModality(Modality.APPLICATION_MODAL)
    dialogStage.title = "$prompt Details"

    val inputFields = fields.map { fieldName ->
        TextField().apply { promptText = fieldName }
    }

    val errorLabel = Label().apply {
        style = "-fx-text-fill: red; -fx-font-size: 12px;"
        isVisible = false
    }

    val confirmButton = Button("Confirm").apply {
        setOnAction {
            val inputs = inputFields.map { it.text.trim() }
            if (inputs.any { it.isEmpty() }) {
                errorLabel.text = "All fields are required."
                errorLabel.isVisible = true
            } else {
                dialogStage.userData = inputs.joinToString(",")
                dialogStage.close()
            }
        }
    }

    val cancelButton = Button("Cancel").apply {
        setOnAction {
            dialogStage.userData = null
            dialogStage.close()
        }
    }

    val inputBoxes = VBox(10.0).apply {
        alignment = Pos.CENTER
        children.addAll(fields.indices.map { VBox(Text(fields[it]), inputFields[it]) })
        children.addAll(errorLabel, HBox(10.0, confirmButton, cancelButton).apply { alignment = Pos.CENTER })
    }

    dialogStage.scene = Scene(inputBoxes, 350.0, 250.0)
    dialogStage.showAndWait()

    return dialogStage.userData as? String
    }
}

fun main()
{
    Application.launch(MainApp::class.java)
}
